#include <iostream>
#include "Interface/IFileReader.h"
#include "Reader/DeMuxer.h"
#include "Reader/FileReader.h"
#include "Reader/AudioDecoder.h"
#include "Reader/VideoDecoder.h"
using namespace std;

namespace av {
class TestFileReader : public IFileReader::Listener {
public:
    void IFileReader::Listener::OnFileReaderNotifyAudioSamples(std::shared_ptr<IAudioSamples> audioSamples) {
        std::cout << "Decode audio data..." << std::endl;
        std::string filePath = "F:/test/1.pcm";
        //std::ofstream outFile(filePath, std::ios::binary);
        std::ofstream outFile(filePath, std::ios::binary | std::ios::app);
        size_t size = audioSamples->pcmData.size() - audioSamples->offset;
        size_t byteSize = size * sizeof(int16_t);
        const char* dataPtr = reinterpret_cast<const char*>(audioSamples->pcmData.data() + audioSamples->offset);
        outFile.write(dataPtr, byteSize);
        outFile.close();
        std::cout << "audio data is saved to:" << filePath << std::endl;
    }

    void IFileReader::Listener::OnFileReaderNotifyVideoFrame(std::shared_ptr<IVideoFrame> videoFrame) {
        static int frameCount = 0; // Used to name each frame, avoid overwriting
        std::cout << "Preparing video data (Frame " << frameCount << ")..." << std::endl;

        // Assuming data is RGBA (4 bytes per pixel) as per IVideoFrame definition
        std::string filePath = "F:/test/video_frame_" + std::to_string(frameCount) + ".rgba";

        std::ofstream outFile(filePath, std::ios::binary);

        size_t frameByteSize = static_cast<size_t>(videoFrame->width) * videoFrame->height * 4;

        std::cout << "Saving RGBA frame. Size: " << videoFrame->width << "x" << videoFrame->height << ", Total Bytes: " << frameByteSize << std::endl;

        outFile.write(reinterpret_cast<const char*>(videoFrame->data.get()), frameByteSize);
        std::cout << "Video data saved to: " << filePath << std::endl;

        outFile.close();
        frameCount++; // Increment frame counter
    }
};
}


int main() {
    cout << "Start decode..." << endl;
    std::shared_ptr<av::FileReader> freader(new av::FileReader());
    freader->Open("F:/test/1.mp4");
    freader->SetListener(new av::TestFileReader());
    cout << "duration is:" << freader->GetDuration() << endl;
    cout << "height * weight = " << freader->GetVideoHeight() << ", " << freader->GetVideoWidth() << std::endl;
    freader->Start();
    //freader->Stop();
    return 0;
}