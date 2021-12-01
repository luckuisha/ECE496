#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <Stk.h>
#include "FileWvOut.h"
#include "fftw3.h"

#include <iostream>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setup_STK();
    setup_FrequencyBox(600);
}

void MainWindow::setup_STK() {
    stk::Stk::setSampleRate(stkFrequency);
}

void MainWindow::setup_FrequencyBox(int initialFrequency) {
    ui->FrequencySlider_1->setMinimum(20);
    ui->FrequencySlider_1->setMaximum(22050);
    ui->FrequencySlider_1->setSingleStep(100);
    ui->FrequencySlider_1->setValue(initialFrequency);

    ui->FrequencySlider_2->setMinimum(20);
    ui->FrequencySlider_2->setMaximum(22050);
    ui->FrequencySlider_2->setSingleStep(100);
    ui->FrequencySlider_2->setValue(initialFrequency);
}

void MainWindow::on_FrequencySlider_1_valueChanged(int value) {
    ui->FrequencyLabel_1->setText(QString::number(value) + "Hz");
    sineWaveFrequency_1 = value;
}
void MainWindow::on_SineButton_1_clicked(bool) {
    TimeDomain* graph = ui->timeDomainInput_1;

    QString currentDirectory = QDir::currentPath();
    QString file = currentDirectory + "/audio_files/gen_sine_1.wav";

    drawWaveFromFile(graph, "");

    generateSineWav(sineWave_1, sineWaveFrequency_1, file);
    drawWaveFromFile(graph, file);
}

void MainWindow::on_FreqDomain_1_clicked(bool) {
    TimeDomain* graph = ui->timeDomainInput_1;

    QString currentDirectory = QDir::currentPath();
    QString file = currentDirectory + "/audio_files/gen_sine_1.wav";

    drawWaveFromFile(graph, "");

    generateSineWav(sineWave_1, sineWaveFrequency_1, file);
    //TODO: Move this all into a cpp in the backend
    //fft here
    mFftIn  = fftw_alloc_real(4410);
    mFftOut = fftw_alloc_real(4410);
    mFftPlan = fftw_plan_r2r_1d(4410, mFftIn, mFftOut, FFTW_R2HC,FFTW_ESTIMATE);

    //file->mFftIn
    for(int i = 0; i < 4410; i++) {
        mFftIn[i] = 0.5*(sineWave_1.tick());
    }
    fftw_execute(mFftPlan);
    //FftOut->file
    stk::FileWvOut output;
    // Open a 16-bit, one-channel WAV formatted output file
    output.openFile(file.toStdString(), 1, stk::FileWrite::FILE_WAV, stk::Stk::STK_SINT16);
    for(int i = 0; i < 4410; i++) {
        output.tick(mFftOut[i]);
    }

    drawWaveFromFile(graph, file);
    //destroy fft in/out here
    fftw_free(mFftIn);
    fftw_free(mFftOut);
}


void MainWindow::on_FrequencySlider_2_valueChanged(int value) {
    ui->FrequencyLabel_2->setText(QString::number(value) + "Hz");
    sineWaveFrequency_2 = value;
}
void MainWindow::on_SineButton_2_clicked(bool) {
    TimeDomain* graph = ui->timeDomainInput_2;

    QString currentDirectory = QDir::currentPath();
    QString file = currentDirectory + "/audio_files/gen_sine_2.wav";

    drawWaveFromFile(graph, "");

    generateSineWav(sineWave_2, sineWaveFrequency_2, file);
    drawWaveFromFile(graph, file);
}

void MainWindow::on_outputButton_clicked(bool) {
    TimeDomain* graph = ui->timeDomainOutput;

    QString currentDirectory = QDir::currentPath();
    QString file = currentDirectory + "/audio_files/gen_sine_output.wav";

    drawWaveFromFile(graph, "");

    generateSuperimposedWav(file);

    drawWaveFromFile(graph, file);
}

void MainWindow::drawWaveFromFile(TimeDomain* graph, QString file)
{
    graph->setSource(file);
    //ui->timeDomainInput->plot();
}

void MainWindow::generateSineWav(stk::SineWave sineWave, int sineWaveFrequency, QString file) {
    const float duration_secs = 0.1;
    const int numSamples = duration_secs * stkFrequency;

    stk::FileWvOut output;
    // Open a 16-bit, one-channel WAV formatted output file
    output.openFile(file.toStdString(), 1, stk::FileWrite::FILE_WAV, stk::Stk::STK_SINT16);

    sineWave.reset();
    sineWave.setFrequency(sineWaveFrequency);
    for(int i = 0; i < numSamples; i++) {
        output.tick( sineWave.tick() );
    }

    output.closeFile();
}

void MainWindow::generateSuperimposedWav(QString file) {
    const float duration_secs = 0.1;
    const int numSamples = duration_secs * stkFrequency;

    stk::FileWvOut output;
    // Open a 16-bit, one-channel WAV formatted output file
    output.openFile(file.toStdString(), 1, stk::FileWrite::FILE_WAV, stk::Stk::STK_SINT16);

    sineWave_1.reset();
    sineWave_1.setFrequency(sineWaveFrequency_1);
    sineWave_2.reset();
    sineWave_2.setFrequency(sineWaveFrequency_2);
    for(int i = 0; i < numSamples; i++) {
        output.tick(sineWave_1.tick() + sineWave_2.tick());
    }

    output.closeFile();
}


MainWindow::~MainWindow()
{
    delete ui;

    //TODO: Should we free here?
    //fftw_free(mFftIn);
    //fftw_free(mFftOut);
    fftw_destroy_plan(mFftPlan);
}
