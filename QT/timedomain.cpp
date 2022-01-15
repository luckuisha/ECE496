#include "timedomain.h"
#include <QAudioDecoder>

TimeDomain::TimeDomain(QWidget *parent)
    : QCustomPlot(parent)
    , decoder(new QAudioDecoder(this))
{
    sampleScale = 1.0;
    currentFile="";
    wavePlot = addGraph();
    this->setInteraction(QCP::iRangeDrag,true);
    this->setInteraction(QCP::iRangeZoom,true);
    this->axisRects().at(0)->setRangeDrag(Qt::Horizontal);
    this->axisRects().at(0)->setRangeZoom(Qt::Horizontal);
    setMinimumHeight(100);
    connect(decoder, SIGNAL(bufferReady()), this, SLOT(setBuffer()));
    connect(decoder, SIGNAL(finished()), this, SLOT(plot()));

}

TimeDomain::~TimeDomain()
{
    samples.clear();
    delete decoder;
    wavePlot->data()->clear();
    this->removePlottable(wavePlot);
}

void TimeDomain::setSource(const QString &fileName)
{
    if(currentFile != fileName)
    {
        currentFile = fileName;
        wavePlot->data()->clear();
        samples.clear();
        decoder->setSource(currentFile);
        decoder->start();
    }
}

void TimeDomain::setBuffer()
{
    //std::cout << "Setting Buffer" << std::endl;
    buffer = decoder->read();
    QAudioFormat format = buffer.format();
    qreal peak = getPeakValue(format);
    sampleRate = format.sampleRate();
    sampleScale = 1.0;

    //CHECK in case of error constData may come in different Types
    const qint16 *data = buffer.constData<qint16>();
    int count = buffer.sampleCount();
    for (int i=0; i<count; i++){
        double val = data[i]/peak;
        samples.append(val);
    }
    //std::cout << "Setting Buffer Exited" << std::endl;
}

// Sample rate in hertz
void TimeDomain::appendSamples(QVector<double> incomingSamples,int sample_rate)
{
    sampleScale = 1.0;
    samples.clear();
    sampleRate = sample_rate;
    for( int i = 0; i< incomingSamples.length(); i++)
    {
        samples.append(incomingSamples[i]);
    }
}

void TimeDomain::plot()
{
    std::cout << "Plotting" << std::endl;
    QVector<double> vec(samples.size());
    std::cout << "Samples size is " << samples.size() << std::endl;
    for (int i=0; i<vec.size(); i++)
        vec[i] = i*sampleScale/sampleRate;
    wavePlot->addData(vec, samples);
    yAxis->setRange(QCPRange(-1, 1));
    if(vec.size() > 0)
    {
        xAxis->setRange(QCPRange(0, vec.last()));
    }
    else
    {
        xAxis->setRange(QCPRange(0,vec.size()));
    }
    replot();
    std::cout << "Plot exited" << std::endl;
}

int TimeDomain::getSampleRate()
{
    return sampleRate;
}

qreal TimeDomain::getPeakValue(const QAudioFormat &format)
{
    qreal peak(0);
    if(format.isValid())
    {
        if(format.sampleFormat() == QAudioFormat::Unknown)
        {
            //no-op
        }
        if(format.sampleFormat() == QAudioFormat::Float)
        {
            peak = 1;
        }
        if(format.sampleFormat() == QAudioFormat::UInt8)
        {
            peak = UINT_MAX;
        }
        if(format.sampleFormat() == QAudioFormat::Int16)
        {
            peak = SHRT_MAX;
        }
        if(format.sampleFormat() == QAudioFormat::Int32)
        {
            peak = INT_MAX;
        }
    }
    return peak;
}
