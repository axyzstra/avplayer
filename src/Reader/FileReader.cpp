#include "FileReader.h"

namespace av {

IFileReader* IFileReader::Create() {
    return new FileReader();
}

FileReader::FileReader() {
    m_deMuxer = std::shared_ptr<IDeMuxer>(IDeMuxer::Create());

    m_audioDecoder = std::shared_ptr<IAudioDecoder>(IAudioDecoder::Create(2, 44100));
    m_videoDecoder = std::shared_ptr<IVideoDecoder>(IVideoDecoder::Create());

    m_deMuxer->SetListener(this);
    m_audioDecoder->SetListener(this);
    m_videoDecoder->SetListener(this);
}

FileReader::~FileReader() {
    m_deMuxer->Pause();
    m_audioDecoder = nullptr;
    m_videoDecoder = nullptr;
    m_deMuxer = nullptr;
}


void FileReader::SeekTo(float progress) {
    if (m_deMuxer) {
        m_deMuxer->SeekTo(progress);
    }
}

void FileReader::Start() {
    if (m_deMuxer) {
        m_deMuxer->Start();
    }
    if (m_audioDecoder) {
        m_audioDecoder->Start();
    }
    if (m_videoDecoder) {
        m_videoDecoder->Start();
    }
}

void FileReader::Pause() {
    if (m_deMuxer) {
        m_deMuxer->Pause();
    }
    if (m_audioDecoder) {
        m_audioDecoder->Pause();
    }
    if (m_videoDecoder) {
        m_videoDecoder->Pause();
    }
}


void FileReader::Stop() {
    if (m_deMuxer) {
        m_deMuxer->Stop();
    }
    if (m_audioDecoder) {
        m_audioDecoder->Stop();
    }
    if (m_videoDecoder) {
        m_videoDecoder->Stop();
    }
}

float FileReader::GetDuration() {
    return m_deMuxer ? m_deMuxer->GetDuration() : 0;
}

int FileReader::GetVideoWidth() {
    return m_videoDecoder ? m_videoDecoder->GetVideoWidth() : 0;
}

int FileReader::GetVideoHeight() {
    return m_videoDecoder ? m_videoDecoder->GetVideoHeight() : 0;
}

void FileReader::OnNotifyAudioStream(struct AVStream* stream) {
    if (m_audioDecoder) {
        m_audioDecoder->SetStream(stream);
    }
}

void FileReader::OnNotifyVideoStream(struct AVStream* stream) {
    if (m_videoDecoder) {
        m_videoDecoder->SetStream(stream);
    }
}

void FileReader::SetListener(IFileReader::Listener* listener) {
    std::lock_guard<std::recursive_mutex> lock(m_listenerMutex);
    m_listener = listener;
}

bool FileReader::Open(const std::string& filePath) {
    if (!m_deMuxer) {
        return false;
    }
    return m_deMuxer->Open(filePath);
}


void FileReader::OnNotifyAudioPacket(std::shared_ptr<IAVPacket> packet) {
    if (m_audioDecoder) {
        m_audioDecoder->Decode(packet);
    }
}


void FileReader::OnNotifyVideoPacket(std::shared_ptr<IAVPacket> packet) {
    if (m_videoDecoder) {
        m_videoDecoder->Decode(packet);
    }
}

void FileReader::OnNotifyAudioSamples(std::shared_ptr<IAudioSamples> audioSamples) {
    std::lock_guard<std::recursive_mutex> lock(m_listenerMutex);
    if (m_listener) {
        m_listener->OnFileReaderNotifyAudioSamples(audioSamples);
    }
}

void FileReader::OnNotifyVideoFrame(std::shared_ptr<IVideoFrame> videoFrame) {
    std::lock_guard<std::recursive_mutex> lock(m_listenerMutex);
    if (m_listener) {
        m_listener->OnFileReaderNotifyVideoFrame(videoFrame);
    }
}


}