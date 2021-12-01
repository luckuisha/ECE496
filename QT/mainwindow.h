#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtMultimedia/QAudioInput>
#include <QBuffer>
#include "qcustomplot.h"
#include <QMainWindow>
#include "timedomain.h"
#include <SineWave.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void drawWaveFromFile(TimeDomain* graph, QString file);

    //Adds the controller which manages interactions on the main window

private:
    //Setup functions
    //Called upon initialization
    void setup_STK();
    void setup_FrequencyBox(int initialFrequency);


private slots:
    // Interaction Functions
    void on_FrequencySlider_1_valueChanged(int value);
    void on_SineButton_1_clicked(bool);

    void on_FrequencySlider_2_valueChanged(int value);
    void on_SineButton_2_clicked(bool);

    void on_outputButton_clicked(bool);

private:
    Ui::MainWindow *ui;

    const float stkFrequency = 44100.0;

    int sineWaveFrequency_1 = 440;
    int sineWaveFrequency_2 = 440;

    stk::SineWave sineWave_1;
    stk::SineWave sineWave_2;

    //Generates a .wav file based on the content of sineWave
    //@param file: loction to store generated .wav file
    void generateSineWav(stk::SineWave sineWave, int sineWaveFrequency, QString file);

    void generateSuperimposedWav(QString file);
};
#endif // MAINWINDOW_H
