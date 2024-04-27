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
#include "ui_mainwindow.h"

#include <QtConcurrent/QtConcurrent>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSettings>

#include "checker.h"
#include "optionsAliases.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, mUi(new Ui::MainWindow)
	, mTasksDir(QDir::currentPath())
	, mLocalSettings(QDir::toNativeSeparators(mTasksDir.absolutePath() + "/checkapp.ini"))
{
	mUi->setupUi(this);

	connect(mUi->backgroundOption, &QGroupBox::toggled, [this](bool state){
		mDirOptions[mTasksPath][backgroundOption] = !state;
		mUi->closeOnSuccessOption->setEnabled(state);
	});

	loadSettings();
	mUi->runCheckButton->setEnabled(!mTasksPath.isEmpty());
}

MainWindow::~MainWindow()
{
	saveSettings();
	delete mUi;
}

void MainWindow::on_runCheckButton_clicked()
{
	auto qrsList = mTasksDir.entryInfoList({"*.qrs"}, QDir::Files);
	if (qrsList.isEmpty()) {
		qrsList = mTasksDir.entryInfoList({"*.tsj"}, QDir::Files);
	}
	if (qrsList.isEmpty()) {
		showNoQrsTsjMessage();
		return;
	}

	auto fields = mFieldsDir.entryInfoList({"*.xml"}, QDir::Files);
	if (fields.isEmpty()) {
		showNoFieldsMessage();
		return;
	}

	Checker checker(mTasksPath);
	checker.reviewTasks(qrsList, fields, mDirOptions[mTasksPath]);
}

void MainWindow::on_chooseField_clicked()
{
	mFieldsDir = chooseDirectoryDialog();
	if (mFieldsDir.entryInfoList({"*.xml"}, QDir::Files).isEmpty()) {
		showNoFieldsMessage();
	}

	auto path = QDir::toNativeSeparators(mFieldsDir.absolutePath());
	mUi->xmlFieldsDir->setText(path);
	mDirOptions[mTasksPath][xmlFieldsDir] = path;
}

void MainWindow::on_openTasks_clicked()
{
	mTasksDir = chooseDirectoryDialog();
	if (mTasksDir.entryInfoList({"*.qrs", "*.tsj"}, QDir::Files).isEmpty()) {
		showNoQrsTsjMessage();
		return;
	}

	mTasksPath = QDir::toNativeSeparators(mTasksDir.absolutePath());
	mUi->currentTasksDir->setText(mTasksPath);

	if (!mDirOptions.contains(mTasksPath)) {
		mDirOptions[mTasksPath] = defaultOptions;
	}

	auto xmlField = mDirOptions[mTasksPath][xmlFieldsDir].toString();
	mFieldsDir = xmlField.isEmpty() ? mTasksDir : QDir(xmlField);
	mDirOptions[mTasksPath][xmlFieldsDir] = xmlField.isEmpty() ? mTasksPath : xmlField;

	resetUiOptions(mDirOptions[mTasksPath]);
	mUi->runCheckButton->setEnabled(true);
}

void MainWindow::on_wPcheckBox_stateChanged(int state)
{
	mDirOptions[mTasksPath][patchWP] = state == Qt::CheckState::Checked;
}

void MainWindow::on_wPPCheckBox_stateChanged(int state)
{
	mDirOptions[mTasksPath][patchField] = state == Qt::CheckState::Checked;
}

void MainWindow::on_resetPCheckBox_stateChanged(int state)
{
	mDirOptions[mTasksPath][resetRP] = state == Qt::CheckState::Checked;
}

void MainWindow::on_showConsoleCheckBox_stateChanged(int state)
{
	mDirOptions[mTasksPath][consoleOption] = state == Qt::CheckState::Checked;
}

void MainWindow::on_closeOnSuccessOption_stateChanged(int state)
{
	mDirOptions[mTasksPath][closeSuccessOption] = state == Qt::CheckState::Checked;
}

QDir MainWindow::chooseDirectoryDialog()
{
	QFileDialog dialog;
	dialog.setFileMode(QFileDialog::Directory);
	dialog.exec();
	return dialog.directory();
}

void MainWindow::resetUiOptions(const QHash<QString, QVariant> &options)
{
	options[closeSuccessOption].toBool() ? mUi->closeOnSuccessOption->setCheckState(Qt::CheckState::Checked)
										 : mUi->closeOnSuccessOption->setCheckState(Qt::CheckState::Unchecked);
	mUi->backgroundOption->setChecked(!options[backgroundOption].toBool());
	options[patchField].toBool() ? mUi->wPPCheckBox->setCheckState(Qt::CheckState::Checked)
										 : mUi->wPPCheckBox->setCheckState(Qt::CheckState::Unchecked);
	options[patchWP].toBool() ? mUi->wPcheckBox->setCheckState(Qt::CheckState::Checked)
										 : mUi->wPcheckBox->setCheckState(Qt::CheckState::Unchecked);
	options[resetRP].toBool() ? mUi->resetPCheckBox->setCheckState(Qt::CheckState::Checked)
										 : mUi->resetPCheckBox->setCheckState(Qt::CheckState::Unchecked);
	options[consoleOption].toBool() ? mUi->resetPCheckBox->setCheckState(Qt::CheckState::Checked)
										 : mUi->resetPCheckBox->setCheckState(Qt::CheckState::Unchecked);
	mUi->xmlFieldsDir->setText(options[xmlFieldsDir].toString());
}

void MainWindow::loadSettings()
{
	QSettings settings(mLocalSettings, QSettings::IniFormat);
	auto groups = settings.childGroups();
	for (auto &&g : groups) {
		QHash <QString, QVariant> options;

		settings.beginGroup(g);
		for (auto &&key : defaultOptions.keys()) {
			options[key] = settings.value(key, defaultOptions[key]);
		}
		settings.endGroup();

		mDirOptions[g] = options;
	}
}

void MainWindow::saveSettings()
{
	QSettings settings(mLocalSettings, QSettings::IniFormat);
	for (auto &&dir: mDirOptions.keys()) {
		settings.beginGroup(dir);
		for (auto &&option: mDirOptions[dir].keys()) {
			settings.setValue(option, mDirOptions[dir][option]);
		}
		settings.endGroup();
	}
}

void MainWindow::showNoQrsTsjMessage()
{
	QMessageBox msgBox;
	msgBox.setText(tr("There is no .qrs or .tsj files in solutions directory."));
	msgBox.exec();
}

void MainWindow::showNoFieldsMessage()
{
	QMessageBox msgBox;
	msgBox.setText(tr("There is no .xml files in fields directory."));
	msgBox.exec();
}
