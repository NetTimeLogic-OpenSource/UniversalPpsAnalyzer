//*****************************************************************************************
// Project:     UniversalPpsAnalyzer
//
// Author:      Sven Meier, NetTimeLogic GmbH
//
// License:     Copyright (c) 2022, NetTimeLogic GmbH, Switzerland, Sven Meier <contact@nettimelogic.com>
//              All rights reserved.
//
//              THIS PROGRAM IS FREE SOFTWARE: YOU CAN REDISTRIBUTE IT AND/OR MODIFY
//              IT UNDER THE TERMS OF THE GNU LESSER GENERAL PUBLIC LICENSE AS
//              PUBLISHED BY THE FREE SOFTWARE FOUNDATION, VERSION 3.
//
//              THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT
//              WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//              MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. SEE THE GNU
//              LESSER GENERAL LESSER PUBLIC LICENSE FOR MORE DETAILS.
//
//              YOU SHOULD HAVE RECEIVED A COPY OF THE GNU LESSER GENERAL PUBLIC LICENSE
//              ALONG WITH THIS PROGRAM. IF NOT, SEE <http://www.gnu.org/licenses/>.
//
//*****************************************************************************************

#ifndef UPA_DEBUGSTREAM_H
#define UPA_DEBUGSTREAM_H

#include <iostream>
#include <streambuf>
#include <string>
#include <QMutex>

#include <QPlainTextEdit>

class Upa_DebugStream : public std::basic_streambuf<char>
{
public:
    Upa_DebugStream(std::ostream &stream, QPlainTextEdit* text_edit) : debug_stream(stream)
    {
        line_nr = 0;
        debug_window = text_edit;
        old_stream_buffer = stream.rdbuf();
        stream.rdbuf(this);
    }
    ~Upa_DebugStream()
    {
        // output anything that is left
        if (!debug_string.empty())
            debug_window->appendPlainText(debug_string.c_str());

        debug_stream.rdbuf(old_stream_buffer);
    }

    unsigned int line_nr;

protected:
    virtual int_type overflow(int_type v)
    {
        access_mutex.lock();

        if (v == '\n')
        {
            QTextCharFormat text_format = debug_window->currentCharFormat();
            text_format.setForeground(Qt::black);

            if (debug_string.find("ERROR")!=std::string::npos)
            {
                text_format.setForeground(Qt::darkRed);
            }
            if (debug_string.find("INFO")!=std::string::npos)
            {
                text_format.setForeground(Qt::darkGreen);
            }
            if (debug_string.find("WARNING")!=std::string::npos)
            {
                text_format.setForeground(Qt::darkYellow);
            }
            if (debug_string.find("VERBOSE")!=std::string::npos)
            {
                text_format.setForeground(Qt::gray);
            }

            debug_window->setCurrentCharFormat(text_format);

            debug_window->appendPlainText(debug_string.c_str());

            // Scroll to bottom of window

            debug_window->moveCursor(QTextCursor::End);
            debug_window->moveCursor(QTextCursor::StartOfLine);

            debug_string.erase(debug_string.begin(), debug_string.end());
        }
        else
            debug_string += v;

        access_mutex.unlock();
        return v;
    }

    virtual std::streamsize xsputn(const char *p, std::streamsize n)
    {
        access_mutex.lock();
        debug_string.append(p, p + n);

        std::string::size_type pos = 0;
        while (pos != std::string::npos)
        {
            pos = debug_string.find('\n');
            if (pos != std::string::npos)
            {
                std::string tmp(debug_string.begin(), debug_string.begin() + pos);

                QTextCharFormat text_format = debug_window->currentCharFormat();
                text_format.setForeground(Qt::black);

                if (tmp.find("ERROR")!=std::string::npos)
                {
                    text_format.setForeground(Qt::darkRed);
                }
                if (tmp.find("INFO")!=std::string::npos)
                {
                    text_format.setForeground(Qt::darkGreen);
                }
                if (tmp.find("WARNING")!=std::string::npos)
                {
                    text_format.setForeground(Qt::darkYellow);
                }
                if (tmp.find("VERBOSE")!=std::string::npos)
                {
                    text_format.setForeground(Qt::gray);
                }

                debug_window->setCurrentCharFormat(text_format);

                debug_window->appendPlainText(tmp.c_str());


                // Scroll to bottom of window

                debug_window->moveCursor(QTextCursor::End);
                debug_window->moveCursor(QTextCursor::StartOfLine);

                debug_string.erase(debug_string.begin(), debug_string.begin() + pos + 1);
            }
        }

        access_mutex.unlock();
        return n;
    }

private:
    std::ostream &debug_stream;
    std::streambuf *old_stream_buffer;
    std::string debug_string;
    QPlainTextEdit* debug_window;
    QMutex access_mutex;
};

#endif // Upa_DebugStream_H
