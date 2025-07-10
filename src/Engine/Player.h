#pragma once
#include "IPlayer.h"
#include "AVSynchronizer.h"
#include "Core/TaskPool.h"
#include "Interface/IAudioPipeline.h"
#include "Interface/IAudioSpeaker.h"
#include "Interface/IFileReader.h"
#include "Interface/IVideoDisplayView.h"
#include "Interface/IVideoPipeline.h"
#include <unordered_set>

namespace av {


class Player : public IPlayer,
               public IFileReader::Listener,
               public AVSynchronizer::Listener,
               public IAudioPipeline::Listener,
               public IVideoPipeline::Listener {

public:
    Player(GLContext& glContext);
    ~Player() override;

    void AttachDisplayView(std::shared_ptr<IVideoDisplayView> displayView) override;
    void DetachDisplayView(std::shared_ptr<IVideoDisplayView> displayView) override;

    void SetPlaybackListener(std::shared_ptr<IPlaybackListener> listener) override;

    bool Open(std::string& filePath) override;
    void Play() override;
    void Pause() override;
    void SeekTo(float progress) override;
    bool IsPlaying() override {
        return m_isPlaying;
    }

    std::shared_ptr<IVideoFilter> AddVideoFilter(VideoFilterType type) override;
    void RemoveVideoFilter(VideoFilterType type) override;


private:
    // 管理任务池专用的 OpenGL Context
    void InitTaskPoolGLContext();
    void DestroyTaskPoolGLContext();

    // 继承自IFileReader::Listener
    void OnFileReaderNotifyAudioSamples(std::shared_ptr<IAudioSamples>) override;
    void OnFileReaderNotifyVideoFrame(std::shared_ptr<IVideoFrame>) override;
    void OnFileReaderNotifyAudioFinished() override;
    void OnFileReaderNotifyVideoFinished() override;

    // 继承自AVSynchronizer::Listener
    void OnAVSynchronizerNotifyAudioSamples(std::shared_ptr<IAudioSamples>) override;
    void OnAVSynchronizerNotifyVideoFrame(std::shared_ptr<IVideoFrame>) override;
    void OnAVSynchronizerNotifyAudioFinished() override;
    void OnAVSynchronizerNotifyVideoFinished() override;

    // 继承自IAudioPipeline::Listener
    void OnAudioPipelineNotifyAudioSamples(std::shared_ptr<IAudioSamples>) override;
    void OnAudioPipelineNotifyFinished() override;

    // 继承自IVideoPipeline::Listener
    void OnVideoPipelineNotifyVideoFrame(std::shared_ptr<IVideoFrame>) override;
    void OnVideoPipelineNotifyFinished() override;

private:
    GLContext m_glContext;
    GLContext m_taskPoolGLContext;

    std::shared_ptr<IPlaybackListener> m_playbackListener;
    std::recursive_mutex m_playbackListenerMutex;

    // 文件读取器
    std::shared_ptr<IFileReader> m_fileReader;

    // 同步
    std::shared_ptr<AVSynchronizer> m_avSynchronizer;

    // 音视频处理
    std::shared_ptr<IAudioPipeline> m_audioPipeline;
    std::shared_ptr<IVideoPipeline> m_videoPipeline;

    // 播放音频
    std::shared_ptr<IAudioSpeaker> m_audioSpeaker;

    // 播放视频
    std::unordered_set<std::shared_ptr<IVideoDisplayView>> m_displayViews;
    std::recursive_mutex m_displayViewsMutex;

    // 用于执行需要GL环境的操作，例如销毁纹理等，即 OpenGL 的操作是单独在一个线程中操作的
    std::shared_ptr<TaskPool> m_taskPool;

    std::atomic<bool> m_isPlaying{false};
    std::atomic<bool> m_isRecording{false};

};


}