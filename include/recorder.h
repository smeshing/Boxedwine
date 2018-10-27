#ifndef __RECORDER_H__
#define __RECORDER_H__

#ifdef BOXEDWINE_RECORDER
class Recorder {
public:
    static void start(std::string directory);
    static Recorder* instance;

    void initCommandLine(std::string root, std::string zip, std::string working, const char **argv, U32 argc);
    void takeScreenShot();
    void onMouseMove(U32 x, U32 y);
    void onMouseButton(U32 down, U32 button, U32 x, U32 y);
    void onKey(U32 key, U32 down);
    void close();

    FILE* file;
    std::string directory;
private:
    void out(const char* s);
    int screenShotCount;
    void fullScrennShot();
    void partialScreenShot(U32 x, U32 y, U32 w, U32 h);
};
#endif

#endif