/* Copyright 2021 CyberTech Labs Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QStyleFactory>
#include <QTranslator>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication a(argc, argv);
	a.setStyle(QStyleFactory::create("Fusion"));

	QTranslator translator;
	translator.load(":/translations/checkapp_" + QLocale::system().name() + ".qm");
	a.installTranslator(&translator);

	MainWindow w;
	w.show();
	return a.exec();
}
