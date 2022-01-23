#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <Stk.h>
#include "FileWvOut.h"
#include "../external/ECE496-Backend/src/wave.h"

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

    // RtAudio API setup
    streamParameters.deviceId = dac.getDefaultInputDevice();
    streamParameters.nChannels = 1;
    audioFormat = ( sizeof(stk::StkFloat) == 8) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;

    bufferFrames = stk::RT_BUFFER_SIZE;
    stk::Stk::setSampleRate(stkFrequency);
}

void MainWindow::setup_FrequencyBox(int initialFrequency) {
    ui->FrequencySlider->setMinimum(20);
    ui->FrequencySlider->setMaximum(22050);
    ui->FrequencySlider->setSingleStep(100);
    ui->FrequencySlider->setValue(initialFrequency);
}

void MainWindow::on_FrequencySlider_valueChanged(int value) {
    ui->FrequencyLabel->setText(QString::number(value) + "Hz");
    sineWaveFrequency = value;
}

void MainWindow::drawWaveFromFile(QString file)
{
    ui->timeDomainInput->setSource(file);
    //ui->timeDomainInput->plot();
}

void MainWindow::on_playButton_clicked(bool)
{

    // Get current cursor position when it exists
    // If this following section is not loading the sound file
    // Ensure that you have the right working directory set under
    // Projects->Run->Working Directory
    QString currentDirectory = QDir::currentPath();
    //std::cout << "Play Button Pushed, currentDir =" << currentDirectory.toStdString() << std::endl;
    QString file = currentDirectory + "/audio_files/ImperialMarch60.wav";
    drawWaveFromFile(file);

    playFile(file);
    //std::cout << "Play Button Finished" << std::endl;
}

void MainWindow::on_playSineButton_clicked(bool) {

    QString currentDirectory = QDir::currentPath();
    QString file = currentDirectory + "/audio_files/gen_sine.wav";

    //Clear the graph so that generateSineWave() is not
    //Accessing the same file
    //TODO: Better method for this
    drawWaveFromFile("");

    generateSineWav(file);
    drawWaveFromFile(file);
    playSine();
}

void MainWindow::generateSineWav(QString file) {
    const float duration_secs = 0.1;
    const int numSamples = duration_secs * stkFrequency;

    stk::FileWvOut output;
    // Open a 16-bit, one-channel WAV formatted output file
    output.openFile(file.toStdString(), 1, stk::FileWrite::FILE_WAV, stk::Stk::STK_SINT16);

    //sineWave.reset();
    //sineWave.setFrequency(sineWaveFrequency);
    wave* wave1 = new wave(1,sineWaveFrequency,0,duration_secs,"saw");
    for(int i = 0; i < numSamples; i++) {
        output.tick(wave1->values[i]);
    }
    //delete wave1;
    //This causes a memory leak lol - will fix later

    output.closeFile();
}

void MainWindow::playFile(QString file){
    if (dac.isStreamOpen()) dac.closeStream();
    if (input.isOpen()) input.closeFile();

    try {
        input.openFile(file.toStdString());
    } catch (stk::StkError &error){
        error.printMessage();
    }

    double rate = input.getFileRate() / stk::Stk::sampleRate();
    input.setRate(rate);

    try {
        dac.openStream( &streamParameters, NULL, audioFormat, stk::Stk::sampleRate(), &bufferFrames, &tickFile, (void *) &input);
    } catch (RtAudioError &error) {
        error.printMessage();
    }

    dac.startStream();
}

void MainWindow::playSine(){
    if (dac.isStreamOpen()) dac.closeStream();
    try {
        dac.openStream( &streamParameters, NULL, audioFormat, stk::Stk::sampleRate(), &bufferFrames, &tickSine, (void *) &sineWave);
    } catch (RtAudioError &error) {
        error.printMessage();
    }

    dac.startStream();
}

void MainWindow::on_stopSound_clicked(bool){
    if (dac.isStreamOpen()) dac.closeStream();
    if (input.isOpen()) input.closeFile();
}

int tickFile( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
    stk::FileWvIn *input = (stk::FileWvIn *) userData;
    auto *samples = (stk::StkFloat *) outputBuffer;
    for ( unsigned int i=0; i<nBufferFrames; i++ ){
        try {
            *samples++ = input->tick();
        }  catch (stk::StkError &error) {
            error.printMessage();
        }
    }
    return 0;
}

int tickSine( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *dataPointer )
{
    stk::SineWave *sine = (stk::SineWave *) dataPointer;
    auto *samples = (stk::StkFloat *) outputBuffer;
    for ( unsigned int i=0; i<nBufferFrames; i++ ){
        try {
            *samples++ = sine->tick();
        } catch (stk::StkError &error) {
            error.printMessage();
        }
    }
    return 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}
