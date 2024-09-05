#include <sstream>
#include <iostream>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

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
        scene->DrawText((char *)userTextStream.str().c_str(), fontTxt, 150, 150, bgColor, fgColor);

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
                    merge_files(files, "/data/pkg/merged_file.pkg")
                } else {
                    listen = true;
                }
            }
        }
    }

    return 0;
}
