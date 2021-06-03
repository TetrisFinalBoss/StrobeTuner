#ifndef AUDIOSTROBE_H
#define AUDIOSTROBE_H

#include <QObject>

#include <QAudioInput>
#include <QBuffer>

#include <memory>

class AudioStrobe : public QObject
{
    Q_OBJECT
public:
    explicit AudioStrobe(QObject *parent = nullptr);

    Q_PROPERTY(qreal threshold MEMBER _threshold)               // Audio level threshold
    Q_PROPERTY(qreal wheelFreq MEMBER _wheel_freq)              // Wheel rotation frequency
    Q_PROPERTY(qreal angle MEMBER _angle NOTIFY angleChanged)   // Strobe highlighted angle wheel
    Q_PROPERTY(qreal peak MEMBER _peak NOTIFY peakChanged)      // Peak volume level in dB (updated periodically)

signals:
    void angleChanged(qreal);
    void peakChanged(qreal);

private slots:
    void read_audio();

private:
    std::shared_ptr<QAudioInput> _input;
    std::shared_ptr<QBuffer> _audio_buf;

    std::vector<float> _unfiltered;
    std::vector<float> _filtered;

    float _threshold = 0.002;
    bool _strobe_state = false;
    quint64 _sample_count = 0;
    double _angle = 0;
    double _peak;

    double _wheel_freq = 97.999;    // G string on bass

    void filter(quint64 sz);

public:
    static const int FILTER_ORDER = 9;
    static const int REFRESH_RATE = 60;
    static const int SAMPLE_RATE = 44100;
    static const int MAX_SAMPLES_COUNT = (2 * SAMPLE_RATE) / REFRESH_RATE;    // Enough to store data for two periods
};

#endif // AUDIOSTROBE_H
