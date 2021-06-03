#include "audiostrobe.h"

#include <QAudioFormat>
#include <QAudioDeviceInfo>

#include <QDebug>

#include <math.h>

AudioStrobe::AudioStrobe(QObject *parent) : QObject(parent)
{
    auto input_dev = QAudioDeviceInfo::defaultInputDevice();

    qDebug() << input_dev.deviceName();

    QAudioFormat fmt;
    fmt.setByteOrder(static_cast<QAudioFormat::Endian>(QSysInfo::ByteOrder)); // Native
    fmt.setChannelCount(1);
    fmt.setCodec("audio/pcm");
    fmt.setSampleRate(SAMPLE_RATE);
    fmt.setSampleSize(32);
    fmt.setSampleType(QAudioFormat::Float);

    // TODO For now we support ONLY 1 channel 32 bit floats in 44.1KHz!!!!11

    if (!input_dev.isFormatSupported(fmt)) {
        throw std::runtime_error(tr("Audio format not supported!").toStdString());
    }

    // Format supported, everything is OK
    _input = std::make_shared<QAudioInput>(input_dev, fmt);
    _input->setNotifyInterval(1000 / REFRESH_RATE);   // 60 times per second
    _input->setVolume(1.0);

    connect(_input.get(), &QAudioInput::notify, this, &AudioStrobe::read_audio);

    _audio_buf = std::make_shared<QBuffer>();
    _audio_buf->open(QIODevice::ReadWrite);
    _audio_buf->seek(0);

    _input->start(_audio_buf.get());

    // Prepare filter buffer large enough to store twice samples for desired period
    _unfiltered.resize(FILTER_ORDER + MAX_SAMPLES_COUNT, 0.0);
    _filtered.resize(FILTER_ORDER + MAX_SAMPLES_COUNT, 0.0);
}

void AudioStrobe::read_audio()
{
    // Read unfiltered data
    static const int data_sz = _input->format().bytesPerFrame();
    static const double sample_dur = 1.0 / (double)SAMPLE_RATE;

    double read_pos, ready_read = 0;
    ready_read = _audio_buf->pos();
    read_pos = 0;

    // Process input samples
    while (ready_read>=data_sz) {
        quint64 samples_to_read = ready_read / data_sz;

        if (samples_to_read > MAX_SAMPLES_COUNT) {
            samples_to_read = MAX_SAMPLES_COUNT;
        }

        _audio_buf->seek(read_pos);
        auto read_size =_audio_buf->read(reinterpret_cast<char *>(&_unfiltered[FILTER_ORDER]), samples_to_read * data_sz);
        ready_read -= read_size;
        read_pos += read_size;

        quint64 samples_read = read_size/data_sz;
        quint64 last_sample = samples_read+FILTER_ORDER;

        double peak_lvl = 0;

        // Apply band pass filters to input
        filter(samples_read);

        // Analyze filtered data and do strobe
        for (int i = FILTER_ORDER; i<last_sample; ++i) {
            float val = _filtered[i];
            if (_strobe_state && val<-_threshold) {
                // We are past -threshold value, reset strobe state
                _strobe_state = false;
            } else if (!_strobe_state && val>_threshold) {
                // We should light the strobe here
                _strobe_state = true;

                auto current_sample = _sample_count+i;

                // How much wheel has traveled during this time relative to wheel rotation frequency
                double wheel_rel_pos = fmod((double)current_sample * sample_dur, 1.0/_wheel_freq);

                // Angle in degrees
                double wheel_angle = wheel_rel_pos * _wheel_freq * 360.0;

                // qDebug() << "Current wheel angle in degrees: " << wheel_rad;

                _angle = wheel_angle;
                emit angleChanged(_angle);
            }

            if (qAbs<float>(val)>peak_lvl) {
                peak_lvl = qAbs<float>(val);
            }
        }

        // Calculate peak level in dB for this block of samples
        _peak = 20 * log10 (peak_lvl / 1.0);
        emit peakChanged(_peak);

        // qDebug() << "Peak level" << _peak << "dB";

        _sample_count += samples_read;
    }
    _audio_buf->seek(0);
}

void AudioStrobe::filter(quint64 sz)
{
    // Internets suggsts this coefficients for 9th order band pass filter with 44100Hz sample rate
    // which consists of 1st order HPF at 30Hz and 8th order type 1 Chebychev LPF at 2Khz with 4% passband

    static const float flt_x[] = {9.684457e-07, 6.779120e-06, 1.936891e-05, 2.711648e-05,
                                  1.355824e-05, -1.355824e-05, -2.711648e-05, -1.936891e-05,
                                  -6.779120e-06, -9.684457e-07};

    static const float flt_y[] = {0, -7.616874e+00, 2.634041e+01, -5.424242e+01,
                                  7.325881e+01, -6.726091e+01, 4.196424e+01, -1.715111e+01,
                                  4.166027e+00, -4.581673e-01};

    // Apply filters
    auto last_sample = sz + FILTER_ORDER;
    for (int i = FILTER_ORDER; i<last_sample; i++) {
        double val = 0.0;
        for (int j=0; j<=FILTER_ORDER; j++) {
            val += flt_x[j] * _unfiltered[i-j] - flt_y[j] * _filtered[i-j];
        }
        _filtered[i] = val;
    }

    // Save last FILTER_ORDER values for next iteration
    for (int i=0; i<FILTER_ORDER; ++i) {
        _unfiltered[i] = _unfiltered[sz+i];
        _filtered[i] = _filtered[sz+i];
    }
}
