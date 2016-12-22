/*
    Vidiot id a VIDeo Input Output Transformer With a Touch of Functionality
    Copyright (C) 2016  Delicode Ltd

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef STDIN_THREAD_H
#define STDIN_THREAD_H

#include "predefines.h"

class StdInListener : public QObject
{
    Q_OBJECT
public:
    StdInListener() : QObject()
    {
        setObjectName("StdInListener");
    }

    QTimer runtimer;
    QElapsedTimer t;

public slots:
    void listen() {
        std::string line_str;

        getline(std::cin, line_str);
        QString line = QByteArray::fromStdString(line_str);

        emit incomingData(line.toLocal8Bit());

        if(line.startsWith("exit")) {
            emit stop();
            qApp->quit();
        }
        else if(line.startsWith("seek:")) {
            line.remove('\r');
            line.remove('\n');

            QString seekstr = line.split(":").last();
            bool float_ok = false;
            float seek_time = seekstr.toFloat(&float_ok);

            if(float_ok)
                emit seek(seek_time);
        }
        else if(line.startsWith("pause")) {
            emit pause(true);
        }
        else if(line.startsWith("play")) {
            emit pause(false);
        }
        else if(line.startsWith("rewind")) {
            emit seek(0);
        }
        else if(line.startsWith("ping")) {
            emit ping();
        }
        else if(line.startsWith("show_properties")) {
            emit showCameraProperties();
        }
    }

signals:
    void incomingData(QByteArray data);
    void pause(bool);
    void seek(qreal);
    void ping();
    void stop();
    void showCameraProperties();
};

class StdInThread : public QThread
{
    Q_OBJECT
signals:


public:
    StdInThread() : QThread() {
        listener.moveToThread(this);
        listener.runtimer.moveToThread(this);

        connect(&listener.runtimer, SIGNAL(timeout()), &listener, SLOT(listen()));
        connect(&listener, SIGNAL(stop()), this, SLOT(quit()));
    }

    StdInListener listener;

    bool running;
    void run(void) {
        listener.runtimer.start(1);

        int ret = exec();

        listener.runtimer.stop();

        std::cout << "In thread stopped" << thread() << std::endl;
     }
};

#endif //STDIN_THREAD_H
