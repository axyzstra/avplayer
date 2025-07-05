#include "Player.h"

namespace av {

void Player::InitTaskPoolGLContext() {
    m_taskPool->SubmitTask([this]() {
        m_taskPoolGLContext.Initialize();
        m_taskPoolGLContext.MakeCurrent();
    });
}

void Player::DestroyTaskPoolGLContext() {
    SyncNotifier notifier;
    m_taskPool->SubmitTask([&]() {
        m_taskPoolGLContext.MakeCurrent();
        m_taskPoolGLContext.Destroy();
        notifier.Notify();
    });
    notifier.Wait();
}


Player::Player(GLContext& glContext) : m_glContext(glContext), m_taskPoolGLContext(glContext) {
    m_taskPool = std::make_shared<TaskPool>();
    InitTaskPoolGLContext();

    m_fileReader = std::shared_ptr<IFileReader>(IFileReader::Create());

    m_avSynchronizer = std::make_shared<AVSynchronizer>(m_glContext);

    m_audioPipeline = std::shared_ptr<IAudioPipeline>(IAudioPipeline::Create(2, 44100));
    m_videoPipeline = std::shared_ptr<IVideoPipeline>(IVideoPipeline::Create(m_glContext));

    m_audioSpeaker= std::shared_ptr<IAudioSpeaker>(IAudioSpeaker::Create(2, 44100));

    m_fileReader->SetListener(this);
    m_avSynchronizer->SetListener(this);
    m_audioPipeline->SetListener(this);
    m_videoPipeline->SetListener(this);
}

Player::~Player() {
    m_fileReader->Stop();
    m_avSynchronizer->Stop();
    m_videoPipeline->Stop();
    m_audioSpeaker->Stop();
    for (auto display : m_displayViews) {
        display->Clear();
    }
    m_displayViews.clear();

    m_fileReader = nullptr;
    m_avSynchronizer = nullptr;
    m_audioPipeline = nullptr;
    m_videoPipeline = nullptr;
    m_audioSpeaker = nullptr;

    DestroyTaskPoolGLContext();
}

bool Player::Open(std::string& filePath) {
    if (!m_fileReader) {
        return false;
    }
    return m_fileReader->Open(filePath);
}

void Player::Play() {
    if (m_fileReader) {
        m_fileReader->Start();
        m_isPlaying = true;
    }
    std::lock_guard<std::recursive_mutex> lock(m_playbackListenerMutex);
    if (m_playbackListener) {
        // 通知开始播放
        m_playbackListener->NotifyPlaybackStarted();
    }
}

void Player::Pause() {
    if (!m_fileReader) {
        return;
    }
    m_fileReader->Pause();
    m_isPlaying = false;
    std::lock_guard<std::recursive_mutex> lock(m_playbackListenerMutex);
    if (m_playbackListener) {
        m_playbackListener->NotifyPlaybackPaused();
    }
}

void Player::SeekTo(float progress) {
    if (IsPlaying()) {
        Pause();
    }
    if (!m_fileReader) {
        return;
    }
    m_fileReader->SeekTo(progress);
    m_avSynchronizer->Reset();
}

void Player::AttachDisplayView(std::shared_ptr<IVideoDisplayView> displayView) {
    std::lock_guard<std::recursive_mutex> lock(m_displayViewsMutex);
    displayView->SetTaskPool(m_taskPool);
    m_displayViews.insert(displayView);
}

void Player::DetachDisplayView(std::shared_ptr<IVideoDisplayView> displayView) {
    std::lock_guard<std::recursive_mutex> lock(m_displayViewsMutex);
    displayView->Clear();
    m_displayViews.erase(displayView);
}

void Player::SetPlaybackListener(std::shared_ptr<IPlaybackListener> listener) {
    std::lock_guard<std::recursive_mutex> lock(m_playbackListenerMutex);
    m_playbackListener = listener;
}

std::shared_ptr<IVideoFilter> Player::AddVideoFilter(VideoFilterType type) {
    return m_videoPipeline ? m_videoPipeline->AddVideoFilter(type) : nullptr;
}

void Player::RemoveVideoFilter(VideoFilterType type) {
    if (m_videoPipeline) m_videoPipeline->RemoveVideoFilter(type);
}


// 继承自IFileReader::Listener
void Player::OnFileReaderNotifyAudioSamples(std::shared_ptr<IAudioSamples> audioSamples) {
    if (m_avSynchronizer) m_avSynchronizer->NotifyAudioSamples(audioSamples);
}

void Player::OnFileReaderNotifyVideoFrame(std::shared_ptr<IVideoFrame> videoFrame) {
    if (m_avSynchronizer) m_avSynchronizer->NotifyVideoFrame(videoFrame);
}

void Player::OnFileReaderNotifyAudioFinished() {
    if (m_avSynchronizer) m_avSynchronizer->NotifyAudioFinished();
}

void Player::OnFileReaderNotifyVideoFinished() {
    if (m_avSynchronizer) m_avSynchronizer->NotifyVideoFinished();
}


// 继承自AVSynchronizer::Listener
void Player::OnAVSynchronizerNotifyAudioSamples(std::shared_ptr<IAudioSamples> audioSamples) {
    if (m_audioPipeline) m_audioPipeline->NotifyAudioSamples(audioSamples);
}

void Player::OnAVSynchronizerNotifyVideoFrame(std::shared_ptr<IVideoFrame> videoFrame) {
    if (m_videoPipeline) m_videoPipeline->NotifyVideoFrame(videoFrame);
}

void Player::OnAVSynchronizerNotifyAudioFinished() {
    if (m_audioPipeline) m_audioPipeline->NotifyAudioFinished();
}

void Player::OnAVSynchronizerNotifyVideoFinished() {
    if (m_videoPipeline) m_videoPipeline->NotifyVideoFinished();
}

// 继承自IAudioPipeline::Listener
void Player::OnAudioPipelineNotifyAudioSamples(std::shared_ptr<IAudioSamples> audioSamples) {
    if (m_audioSpeaker) m_audioSpeaker->PlayAudioSamples(audioSamples);

    std::lock_guard<std::recursive_mutex> lock(m_playbackListenerMutex);
    if (m_playbackListener) {
        m_playbackListener->NotifyPlaybackTimeChanged(audioSamples->GetTimeStamp(), m_fileReader->GetDuration());
    }
}

void Player::OnAudioPipelineNotifyFinished() {
    {
        std::lock_guard<std::recursive_mutex> lock(m_playbackListenerMutex);
        if (m_playbackListener) m_playbackListener->NotifyPlaybackEOF();
    }
}

// 继承自IVideoPipeline::Listener
void Player::OnVideoPipelineNotifyVideoFrame(std::shared_ptr<IVideoFrame> videoFrame) {
    for (const auto& display : m_displayViews) {
        display->Render(videoFrame, IVideoDisplayView::EContentMode::kScaleAspectFit);
    }
}

void Player::OnVideoPipelineNotifyFinished() {
}

}