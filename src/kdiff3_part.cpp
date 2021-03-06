/***************************************************************************
 * Copyright (C) 2003-2007 Joachim Eibl <joachim.eibl at gmx.de>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "kdiff3_part.h"
#include "fileaccess.h"
#include "kdiff3.h"

#include <QFile>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextStream>

#include <KAboutData>
#include <KLocalizedString>
#include <KMessageBox>
#include <KParts/MainWindow>

#include <version.h>

static KAboutData createAboutData()
{
    QString appVersion = QString(KDIFF3_VERSION_STRING);
    if(sizeof(void*) == 8)
        appVersion += " (64 bit)";
    else if(sizeof(void*) == 4)
        appVersion += " (32 bit)";

    KAboutData aboutData(QLatin1String("kdiff3part"), i18n("KDiff3 Part"),
                         appVersion, i18n("A KPart to display SVG images"),
                         KAboutLicense::GPL_V2,
                         i18n("Copyright 2007, Aurélien Gâteau <aurelien.gateau@free.fr>"));
    aboutData.addAuthor(i18n("Joachim Eibl"), QString(), QString("joachim.eibl at gmx.de"));
    return aboutData;
}

K_PLUGIN_FACTORY(KDiff3PartFactory, registerPlugin<KDiff3Part>();)
//K_EXPORT_PLUGIN( KDiff3PartFactory(createAboutData()))

KDiff3Part::KDiff3Part(QWidget* parentWidget, QObject* parent, const QVariantList& args)
    : KParts::ReadWritePart(parent)
{
    //set AboutData
    setComponentData(createAboutData());
    const QString widgetName = args[0].toString();

    // this should be your custom internal widget
    m_widget = new KDiff3App(parentWidget, widgetName, this);

    //FIXME: This hack is necessary to avoid a crash when the program terminates.
    m_bIsShell = qobject_cast<KParts::MainWindow*>(parentWidget) != nullptr;

    // notify the part that this is our internal widget
    setWidget(m_widget);

    // create our actions
    //KStandardAction::open(this, &KDiff3Part:fileOpen, actionCollection());
    //KStandardAction::saveAs(this, &KDiff3Part:fileSaveAs, actionCollection());
    //KStandardAction::save(this, &KDiff3Part:save, actionCollection());

    setXMLFile("kdiff3_part.rc");

    // we are read-write by default
    setReadWrite(true);

    // we are not modified since we haven't done anything yet
    setModified(false);
}

KDiff3Part::~KDiff3Part()
{
    if(m_widget != nullptr && !m_bIsShell)
    {
        m_widget->saveOptions(KSharedConfig::openConfig());
    }
}

void KDiff3Part::setReadWrite(bool /*rw*/)
{
    //    ReadWritePart::setReadWrite(rw);
}

void KDiff3Part::setModified(bool /*modified*/)
{
    /*
    // get a handle on our Save action and make sure it is valid
    QAction *save = actionCollection()->action(KStandardAction::stdName(KStandardAction::Save));
    if (!save)
        return;

    // if so, we either enable or disable it based on the current
    // state
    if (modified)
        save->setEnabled(true);
    else
        save->setEnabled(false);

    // in any event, we want our parent to do it's thing
    ReadWritePart::setModified(modified);
*/
}

static void getNameAndVersion(const QString& str, const QString& lineStart, QString& fileName, QString& version)
{
    if(str.left(lineStart.length()) == lineStart && fileName.isEmpty())
    {
        int pos = lineStart.length();
        while(pos < str.length() && (str[pos] == ' ' || str[pos] == '\t')) ++pos;
        int pos2 = str.length() - 1;
        while(pos2 > pos)
        {
            while(pos2 > pos && str[pos2] != ' ' && str[pos2] != '\t') --pos2;
            fileName = str.mid(pos, pos2 - pos);
            fprintf(stderr, "KDiff3: %s\n", fileName.toLatin1().constData());
            if(FileAccess(fileName).exists()) break;
            --pos2;
        }

        int vpos = str.lastIndexOf("\t", -1);
        if(vpos > 0 && vpos > (int)pos2)
        {
            version = str.mid(vpos + 1);
            while(!version.right(1)[0].isLetterOrNumber())
                version.truncate(version.length() - 1);
        }
    }
}

bool KDiff3Part::openFile()
{
    // m_file is always local so we can use QFile on it
    fprintf(stderr, "KDiff3: %s\n", localFilePath().toLatin1().constData());
    QFile file(localFilePath());
    if(!file.open(QIODevice::ReadOnly))
        return false;

    // our example widget is text-based, so we use QTextStream instead
    // of a raw QDataStream
    QTextStream stream(&file);
    QString str;
    QString fileName1;
    QString fileName2;
    QString version1;
    QString version2;
    while(!stream.atEnd() && (fileName1.isEmpty() || fileName2.isEmpty()))
    {
        str = stream.readLine() + '\n';
        getNameAndVersion(str, "---", fileName1, version1);
        getNameAndVersion(str, "+++", fileName2, version2);
    }

    file.close();

    if(fileName1.isEmpty() && fileName2.isEmpty())
    {
        KMessageBox::sorry(m_widget, i18n("Could not find files for comparison."));
        return false;
    }

    FileAccess f1(fileName1);
    FileAccess f2(fileName2);

    if(f1.exists() && f2.exists() && fileName1 != fileName2)
    {
        m_widget->slotFileOpen2(fileName1, fileName2, "", "", "", "", "", nullptr);
        return true;
    }
    else if(version1.isEmpty() && f1.exists())
    {
        // Normal patch
        // patch -f -u --ignore-whitespace -i [inputfile] -o [outfile] [patchfile]
        QTemporaryFile tmpFile;
        FileAccess::createTempFile(tmpFile);
        QString tempFileName = tmpFile.fileName();
        QString cmd = "patch -f -u --ignore-whitespace -i \"" + localFilePath() +
                      "\" -o \"" + tempFileName + "\" \"" + fileName1 + "\"";

        QProcess process;
        process.start(cmd);
        process.waitForFinished(-1);

        m_widget->slotFileOpen2(fileName1, tempFileName, "", "",
                                "", version2.isEmpty() ? fileName2 : "REV:" + version2 + ':' + fileName2, "", nullptr); // alias names                                                                                              //    std::cerr << "KDiff3: f1:" << fileName1.toLatin1() <<"<->"<<tempFileName.toLatin1()<< std::endl;
    }
    else if(version2.isEmpty() && f2.exists())
    {
        // Reverse patch
        // patch -f -u -R --ignore-whitespace -i [inputfile] -o [outfile] [patchfile]
        QTemporaryFile tmpFile;
        FileAccess::createTempFile(tmpFile);
        QString tempFileName = tmpFile.fileName();
        QString cmd = "patch -f -u -R --ignore-whitespace -i \"" + localFilePath() +
                      "\" -o \"" + tempFileName + "\" \"" + fileName2 + "\"";

        QProcess process;
        process.start(cmd);
        process.waitForFinished(-1);

        m_widget->slotFileOpen2(tempFileName, fileName2, "", "",
                                version1.isEmpty() ? fileName1 : "REV:" + version1 + ':' + fileName1, "", "", nullptr); // alias name
                                                                                                                  //    std::cerr << "KDiff3: f2:" << fileName2.toLatin1() <<"<->"<<tempFileName.toLatin1()<< std::endl;
    }
    else if(!version1.isEmpty() && !version2.isEmpty())
    {
        fprintf(stderr, "KDiff3: f1/2:%s<->%s\n", fileName1.toLatin1().constData(), fileName2.toLatin1().constData());
        // Assuming that files are on CVS: Try to get them
        // cvs update -p -r [REV] [FILE] > [OUTPUTFILE]

        QTemporaryFile tmpFile1;
        FileAccess::createTempFile(tmpFile1);
        QString tempFileName1 = tmpFile1.fileName();
        QString cmd1 = "cvs update -p -r " + version1 + " \"" + fileName1 + "\" >\"" + tempFileName1 + "\"";
        QProcess process1;
        process1.start(cmd1);
        process1.waitForFinished(-1);

        QTemporaryFile tmpFile2;
        FileAccess::createTempFile(tmpFile2);
        QString tempFileName2 = tmpFile2.fileName();
        QString cmd2 = "cvs update -p -r " + version2 + " \"" + fileName2 + "\" >\"" + tempFileName2 + "\"";
        QProcess process2;
        process2.start(cmd2);
        process2.waitForFinished(-1);

        m_widget->slotFileOpen2(tempFileName1, tempFileName2, "", "",
                                "REV:" + version1 + ':' + fileName1,
                                "REV:" + version2 + ':' + fileName2,
                                "", nullptr);

        //    std::cerr << "KDiff3: f1/2:" << tempFileName1.toLatin1() <<"<->"<<tempFileName2.toLatin1()<< std::endl;
        return true;
    }
    else
    {
        KMessageBox::sorry(m_widget, i18n("Could not find files for comparison."));
    }

    return true;
}

bool KDiff3Part::saveFile()
{
    /*    // if we aren't read-write, return immediately
    if (isReadWrite() == false)
        return false;

    // localFilePath() is always local, so we use QFile
    QFile file(localFilePath());
    if (file.open(IO_WriteOnly) == false)
        return false;

    // use QTextStream to dump the text to the file
    QTextStream stream(&file);
    //stream << m_widget->text();

    file.close();
    return true;
*/
    return false; // Not implemented
}

#include "kdiff3_part.moc"
