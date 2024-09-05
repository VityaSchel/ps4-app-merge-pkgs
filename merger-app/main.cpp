#include <sstream>
#include <iostream>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <thread>
#include <iomanip>
#include <mutex>

#include <orbis/libkernel.h>
#include <orbis/CommonDialog.h>
#include <orbis/MsgDialog.h>
#include <orbis/Sysmodule.h>
#include <orbis/UsbStorage.h>

#define MDIALOG_OK       0
#define MDIALOG_YESNO    1

#include "common/log.h"
#include "common/graphics.cpp"
#include "controller.h"

std::stringstream debugLogStream;

#define FRAME_WIDTH     1920
#define FRAME_HEIGHT    1080
#define FRAME_DEPTH        4

#define FONT_SIZE   	   32

Color bgColor;
Color fgColor;
FT_Face fontTxt;
int frameID = 0;

std::stringstream userTextStream;

std::thread mergeThread;
std::mutex logMutex;

// =================================================================================================

static inline void _orbisCommonDialogSetMagicNumber(uint32_t* magic, const OrbisCommonDialogBaseParam* param)
{
    *magic = (uint32_t)(ORBIS_COMMON_DIALOG_MAGIC_NUMBER + (uint64_t)param);
}

static inline void _orbisCommonDialogBaseParamInit(OrbisCommonDialogBaseParam *param)
{
    memset(param, 0x0, sizeof(OrbisCommonDialogBaseParam));
    param->size = (uint32_t)sizeof(OrbisCommonDialogBaseParam);
    _orbisCommonDialogSetMagicNumber(&(param->magic), param);
}

static inline void orbisMsgDialogParamInitialize(OrbisMsgDialogParam *param)
{
    memset(param, 0x0, sizeof(OrbisMsgDialogParam));
    _orbisCommonDialogBaseParamInit(&param->baseParam);
    param->size = sizeof(OrbisMsgDialogParam);
}

int show_dialog(int dialog_type, const char * format, ...)
{
    OrbisMsgDialogParam param;
    OrbisMsgDialogUserMessageParam userMsgParam;
    OrbisMsgDialogResult result;

    char str[0x800];
    memset(str, 0, sizeof(str));

    va_list opt;
    va_start(opt, format);
    vsprintf(str, format, opt);
    va_end(opt);

    sceMsgDialogInitialize();
    orbisMsgDialogParamInitialize(&param);
    param.mode = ORBIS_MSG_DIALOG_MODE_USER_MSG;

    memset(&userMsgParam, 0, sizeof(userMsgParam));
    userMsgParam.msg = str;
    userMsgParam.buttonType = (dialog_type ? ORBIS_MSG_DIALOG_BUTTON_TYPE_YESNO_FOCUS_NO : ORBIS_MSG_DIALOG_BUTTON_TYPE_OK);
    param.userMsgParam = &userMsgParam;

    if (sceMsgDialogOpen(&param) < 0)
        return 0;

    do { } while (sceMsgDialogUpdateStatus() != ORBIS_COMMON_DIALOG_STATUS_FINISHED);
    sceMsgDialogClose();

    memset(&result, 0, sizeof(result));
    sceMsgDialogGetResult(&result);
    sceMsgDialogTerminate();

    return (result.buttonId == ORBIS_MSG_DIALOG_BUTTON_ID_YES);
}

std::vector<std::string> list_files(const char* usb_path)
{
    std::vector<std::string> file_list;
    DIR* dir;
    struct dirent* entry;
    struct stat file_stat;

    dir = opendir(usb_path);
    if (dir == NULL)
    {
        userTextStream << "Failed to open " << usb_path << " directory\n";
        return file_list;
    }

    userTextStream << "Opened " << usb_path << " directory\n";

    while ((entry = readdir(dir)) != NULL)
    {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", usb_path, entry->d_name);

        if (stat(full_path, &file_stat) == 0 && S_ISREG(file_stat.st_mode))
        {
            file_list.push_back(entry->d_name);
        }
    }

    closedir(dir);
    return file_list;
}

bool compareFilesBySuffix(const std::string& a, const std::string& b)
{
    std::string suffixA = a.substr(a.size() - 12);
    std::string suffixB = b.substr(b.size() - 12);

    if (suffixA.size() != suffixB.size()) return suffixA.size() < suffixB.size();
    return suffixA < suffixB;
}

std::uint64_t get_file_size(const std::string& file_path)
{
    struct stat file_stat;
    if (stat(file_path.c_str(), &file_stat) != 0)
    {
        return 0;
    }
    return static_cast<std::uint64_t>(file_stat.st_size);
}

std::string get_base_filename(const std::string& file_name)
{
    size_t pos = file_name.find_last_of('_');
    if (pos == std::string::npos)
    {
        pos = file_name.find_last_of('.');
    }
    if (pos != std::string::npos)
    {
        return file_name.substr(0, pos);
    }
    return file_name;
}

std::string formatTime(std::uint64_t seconds)
{
    std::uint64_t hours = seconds / 3600;
    std::uint64_t minutes = (seconds % 3600) / 60;
    std::uint64_t secs = seconds % 60;

    std::ostringstream oss;

    if (hours > 0)
    {
        oss << hours << ":";
    }

    oss << std::setfill('0') << std::setw(2) << minutes << ":"
        << std::setfill('0') << std::setw(2) << secs;

    return oss.str();
}

void merge_files(const std::vector<std::string>& files, const std::string& output_path)
{
    std::ofstream outputFile(output_path, std::ios::binary);

    if (!outputFile.is_open())
    {
        std::lock_guard<std::mutex> guard(logMutex);
        userTextStream << "Failed to open output file for writing\n";
        return;
    }

    std::uint64_t totalSize = 0;
    for (const auto& file : files)
    {
        std::string fullPath = "/data/pkg_merger/" + file;
        totalSize += get_file_size(fullPath);
    }

    std::uint64_t processedSize = 0;
    const std::uint64_t reportInterval = totalSize / 10; // Report every 10%
    const std::uint64_t speed = 60 * 1024 * 1024; // 60 mb/s
    std::uint64_t estimatedTimeInSeconds = totalSize / speed;

    {
        std::lock_guard<std::mutex> guard(logMutex);
        userTextStream << "Estimated time: " << formatTime(estimatedTimeInSeconds) << "\n";
    }

    for (const auto& file : files)
    {
        std::string fullPath = "/data/pkg_merger/" + file;

        std::ifstream inputFile(fullPath, std::ios::binary);
        if (!inputFile.is_open())
        {
            std::lock_guard<std::mutex> guard(logMutex);
            userTextStream << "Failed to open input file: " << fullPath << "\n";
            continue;
        }

        {
            std::lock_guard<std::mutex> guard(logMutex);
            userTextStream << "Merging " << file << "\n";
        }

        char buffer[1024 * 1024]; // 1 MB
        while (inputFile.read(buffer, sizeof(buffer)))
        {
            outputFile.write(buffer, inputFile.gcount());
            processedSize += inputFile.gcount();
            if (processedSize >= reportInterval * (processedSize / reportInterval))
            {
                std::lock_guard<std::mutex> guard(logMutex);
                userTextStream << "Progress: " << (processedSize * 100 / totalSize) << "%\n";
            }
        }
        outputFile.write(buffer, inputFile.gcount());
        processedSize += inputFile.gcount();

        if (processedSize >= reportInterval * (processedSize / reportInterval))
        {
            std::lock_guard<std::mutex> guard(logMutex);
            userTextStream << "Progress: " << (processedSize * 100 / totalSize) << "%\n";
        }

        inputFile.close();
    }

    outputFile.close();
    {
        std::lock_guard<std::mutex> guard(logMutex);
        userTextStream << "Files successfully merged into " << output_path << "\n";
    }
}

int main(void)
{   
    setvbuf(stdout, NULL, _IONBF, 0);

    if (sceSysmoduleLoadModule(ORBIS_SYSMODULE_MESSAGE_DIALOG) < 0 ||
        sceCommonDialogInitialize() < 0)
    {
        printf("Failed to initialize CommonDialog\n");
        for(;;);
    }

    int rc;
    int video;
    int curFrame = 0;
    
    DEBUGLOG << "Creating a scene";
    
    auto scene = new Scene2D(FRAME_WIDTH, FRAME_HEIGHT, FRAME_DEPTH);
    
    if(!scene->Init(0xC000000, 2))
    {
        DEBUGLOG << "Failed to initialize 2D scene";
        for(;;);
    }

    bgColor = { 61, 116, 231 };
    fgColor = { 255, 255, 255 };

    const char *font = "/app0/assets/fonts/Montserrat-Regular.ttf";
    
    DEBUGLOG << "Initializing font (" << font << ")";

    if(!scene->InitFont(&fontTxt, font, FONT_SIZE))
    {
        DEBUGLOG << "Failed to initialize font '" << font << "'";
        for(;;);
    }

    userTextStream << "Welcome! Searching in /data/pkg_merger...\n";
    std::vector<std::string> files = list_files("/data/pkg_merger");
    userTextStream << "Found " << files.size() << " files.\n";

    std::sort(files.begin(), files.end(), compareFilesBySuffix);

    for (const auto &file : files)
    {
        if (file.size() > 8 && file.substr(file.size() - 8) == ".pkgpart")
        {
            userTextStream << file << "\n";
        }
    }

    auto controller = new Controller();
    bool listen = false;
    if (files.size() > 1)
    {
        userTextStream << "Press any button on controller to merge parts\n";
        if (!controller->Init(-1))
        {
            userTextStream << "Couldn't initialize controller\n";
            for (;;);
        }

        listen = true;
    }
    else
    {
        userTextStream << "You must split your pkgs on PC and move them from /mnt/usb0 to /data/pkg_merger using FTP\n";
    }
    
    for (;;)
    {
        std::string currentText;

        {
            std::lock_guard<std::mutex> guard(logMutex);
            currentText = userTextStream.str();
        }

        scene->DrawText((char *)currentText.c_str(), fontTxt, 150, 150, bgColor, fgColor);

        // Submit the frame buffer
        scene->SubmitFlip(frameID);
        scene->FrameWait(frameID);

        // Swap to the next buffer
        scene->FrameBufferSwap();
        frameID++;

        if (listen)
        {
            if (controller->TrianglePressed() 
                || controller->CirclePressed() 
                || controller->XPressed() 
                || controller->SquarePressed()
                || controller->L1Pressed()
                || controller->L2Pressed()
                || controller->R1Pressed()
                || controller->R2Pressed()
                || controller->L3Pressed()
                || controller->R3Pressed()
                || controller->StartPressed()
                || controller->DpadUpPressed()
                || controller->DpadRightPressed()
                || controller->DpadDownPressed()
                || controller->DpadLeftPressed()
                || controller->TouchpadPressed()
            )
            {
                listen = false;
                if (show_dialog(MDIALOG_OK, "Press OK to start merging or exit app now"))
                {
                    userTextStream << "Starting merging...\n";
                    
                    mergeThread = std::thread([&]() {
                        std::string baseFileName = get_base_filename(files.front());
                        std::string outputPath = "/data/pkg/" + baseFileName + ".pkg";
                        merge_files(files, outputPath);
                    });
                } else {
                    listen = true;
                }
            }
        }
    }

    return 0;
}
