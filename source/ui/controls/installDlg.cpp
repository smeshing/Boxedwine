#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../../lib/imgui/addon/imguitinyfiledialogs.h"

InstallDlg::InstallDlg() : BaseDlg(INSTALLDLG_TITLE, 600, 300), errorMsg(NULL) {
    this->installLabelText = getTranslation(INSTALLDLG_INSTALL_TYPE_LABEL);
    this->containerLabelText = getTranslation(INSTALLDLG_CONTAINER_LABEL);
    this->locationLabelText = getTranslation(INSTALLDLG_SETUP_FILE_LOCATION_LABEL);
    this->browseButtonText = getTranslation(GENERIC_BROWSE_BUTTON);
    this->installTypeHelp = getTranslation(INSTALLDLG_INSTALL_TYPE_HELP, false);
    this->containerHelp = getTranslation(INSTALLDLG_CONTAINER_HELP, false);
    this->containerNameHelp = getTranslation(INSTALLDLG_CONTAINER_NAME_HELP, false);
    this->wineVersionHelp = getTranslation(COMMON_WINE_VERSION_HELP, false);
    this->wineConfigHelp = getTranslation(INSTALLDLG_RUN_WINE_CONFIG_HELP, false);

    this->leftColumnWidth = ImGui::CalcTextSize(this->installLabelText);
    this->rightColumnWidth = ImGui::CalcTextSize(this->browseButtonText);
    this->rightColumnWidth.x+=8; // more space for button;

    if (this->locationLabelText && this->locationLabelText[0]) {
        ImVec2 locationLabelWidth = ImGui::CalcTextSize(this->locationLabelText);
        if (locationLabelWidth.x>this->leftColumnWidth.x) {
            this->leftColumnWidth = locationLabelWidth;
        }
    }
    locationLabelText = getTranslation(INSTALLDLG_DIRECTORY_LABEL);
    if (this->locationLabelText && this->locationLabelText[0]) {
        ImVec2 locationLabelWidth = ImGui::CalcTextSize(this->locationLabelText);
        if (locationLabelWidth.x>this->leftColumnWidth.x) {
            this->leftColumnWidth = locationLabelWidth;
        }
    }

    ImVec2 containerLabelWidth = ImGui::CalcTextSize(containerLabelText);
    if (containerLabelWidth.x>this->leftColumnWidth.x) {
        this->leftColumnWidth = containerLabelWidth;
    }

    this->leftColumnWidth.x+=COLUMN_PADDING;
    this->rightColumnWidth.x+=COLUMN_PADDING;
            
    this->containerName[0]=0;
    this->locationBuffer[0]=0;

    this->containerComboboxData.data.clear();
    this->containerComboboxData.data.push_back("Create New Container (Recommended)");
    for (auto& container : BoxedwineData::getContainers()) {
        this->containerComboboxData.data.push_back(container->getName());
    }
    this->containerComboboxData.dataChanged();
    this->containerComboboxData.currentSelectedIndex=0;
            
    this->wineVersionComboboxData.data.clear();
    for (auto& ver : GlobalSettings::getWineVersions()) {
        this->wineVersionComboboxData.data.push_back(ver.name);
    }
    this->wineVersionComboboxData.dataChanged();
    this->wineVersionComboboxData.currentSelectedIndex=0;

    this->runWineConfig = false;
}

#define INSTALL_TYPE_SETUP 0
#define INSTALL_TYPE_DIR 1
#define INSTALL_TYPE_MOUNT 2
#define INSTALL_TYPE_BLANK 3

void InstallDlg::run() {
    if (installTypeComboboxData.currentSelectedIndex==INSTALL_TYPE_SETUP) {
        locationLabelText = getTranslation(INSTALLDLG_SETUP_FILE_LOCATION_LABEL);
    } else if (installTypeComboboxData.currentSelectedIndex==INSTALL_TYPE_DIR || installTypeComboboxData.currentSelectedIndex==INSTALL_TYPE_MOUNT) {
        locationLabelText = getTranslation(INSTALLDLG_DIRECTORY_LABEL);
    }
        
    if (this->lastInstallType!=this->installTypeComboboxData.currentSelectedIndex) {
        this->lastInstallType = this->installTypeComboboxData.currentSelectedIndex;
        this->locationBuffer[0] = 0;
    }
        
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing/2));
    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->installLabelText);
    ImGui::SameLine(this->leftColumnWidth.x);
    ImGui::PushItemWidth(-1-(this->installTypeHelp?this->toolTipWidth:0));
    ImGui::Combo("##InstallTypeCombo", &this->installTypeComboboxData.currentSelectedIndex, "Install using a setup program\0Install by copying a directory\0Install by mounting a directory\0Create a blank container\0\0");
    ImGui::PopItemWidth();        
    if (this->installTypeHelp) {
        ImGui::SameLine();
        toolTip(this->installTypeHelp);
    }
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));

    if (this->locationLabelText && this->locationLabelText[0]) {
        const char* locationHelp = NULL;
        int locationHelpId = 0;

        if (this->installTypeComboboxData.currentSelectedIndex==INSTALL_TYPE_SETUP) {
            locationHelpId = INSTALLDLG_TYPE_SETUP_HELP;
        } else if (this->installTypeComboboxData.currentSelectedIndex==INSTALL_TYPE_DIR) {
            locationHelpId = INSTALLDLG_TYPE_DIR_HELP;
        } else if (this->installTypeComboboxData.currentSelectedIndex==INSTALL_TYPE_MOUNT) {
            locationHelpId = INSTALLDLG_TYPE_MOUNT_HELP;
        }
        if (locationHelpId) {
            locationHelp = getTranslation(locationHelpId);
            if (locationHelp && locationHelp[0]==0) {
                locationHelp = NULL;
            }
        }
        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(this->locationLabelText);
        ImGui::SameLine(this->leftColumnWidth.x);
        ImGui::PushItemWidth(-this->rightColumnWidth.x-(locationHelp?this->toolTipWidth:0));
        ImGui::InputText("##LocationID", this->locationBuffer, sizeof(this->locationBuffer));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button(this->browseButtonText)) {
            if (this->installTypeComboboxData.currentSelectedIndex==INSTALL_TYPE_SETUP) {
                const char* types[] = {"*.exe"};
                const char* result = tfd::openFileDialog(getTranslation(INSTALLDLG_OPEN_SETUP_FILE_TITLE), this->locationBuffer, 1, types, NULL, 0);
                if (result) {
                    strcpy(this->locationBuffer, result);
                }
            } else {
                const char* result = tfd::selectFolderDialog(getTranslation(INSTALLDLG_OPEN_FOLDER_TITLE), this->locationBuffer);
                if (result) {
                    strcpy(this->locationBuffer, result);
                }
            }
        }            
        if (locationHelp) {
            ImGui::SameLine();
            this->toolTip(getTranslation(locationHelpId));
        }
        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    }            
                
    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->containerLabelText);
    ImGui::SameLine(this->leftColumnWidth.x);
    ImGui::PushItemWidth(-1-(this->containerHelp?this->toolTipWidth:0));
    ImGui::Combo("##ContainerCombo", &this->containerComboboxData.currentSelectedIndex, this->containerComboboxData.dataForCombobox);
    ImGui::SameLine();
    ImGui::PopItemWidth();
    if (this->containerHelp) {
        ImGui::SameLine();
        this->toolTip(this->containerHelp);
    }
    if (this->containerComboboxData.currentSelectedIndex==0) { // create new container
        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
        SAFE_IMGUI_TEXT("");
        ImGui::SameLine(this->leftColumnWidth.x);
        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(getTranslation(INSTALLDLG_CONTAINER_NAME_LABEL));
        ImGui::SameLine();
        ImGui::PushItemWidth(-1-(this->containerNameHelp?this->toolTipWidth:0));
        ImGui::InputText("##ContainerName", this->containerName, sizeof(this->containerName));
        if (containerNameHelp) {
            ImGui::SameLine();
            this->toolTip(containerNameHelp);
        }

        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
        SAFE_IMGUI_TEXT("");
        ImGui::SameLine(this->leftColumnWidth.x);
        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(getTranslation(COMMON_WINE_VERSION_LABEL));
        ImGui::SameLine();
        ImGui::PushItemWidth(-1-(this->wineVersionHelp?this->toolTipWidth:0));
        ImGui::Combo("##WineCombo", &this->wineVersionComboboxData.currentSelectedIndex, this->wineVersionComboboxData.dataForCombobox);
        ImGui::PopItemWidth();        
        if (this->wineVersionHelp) {
            ImGui::SameLine();
            this->toolTip(wineVersionHelp);
        }                        

        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
        SAFE_IMGUI_TEXT("");
        ImGui::SameLine(leftColumnWidth.x);
        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(getTranslation(INSTALLDLG_CONTAINER_RUN_WINE_CONFIG_LABEL));
        ImGui::SameLine();
        ImGui::Checkbox("##WineConfigCheckbox", &runWineConfig);
        if (wineConfigHelp) {
            ImGui::SameLine();
            this->toolTip(wineConfigHelp);
        }           
    }
    this->addOkAndCancelButtons();
}

void InstallDlg::onOk(bool buttonClicked) {    
    if (buttonClicked) {
        if (this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_SETUP) {
            if (strlen(this->locationBuffer)==0) {
                this->errorMsg = getTranslation(INSTALLDLG_ERROR_SETUP_FILE_MISSING);
            } else if (!Fs::doesNativePathExist(this->locationBuffer)) {
                this->errorMsg = getTranslation(INSTALLDLG_ERROR_SETUP_FILE_NOT_FOUND);
            }            
        } else if (this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_DIR || this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_MOUNT) {
            if (strlen(this->locationBuffer)==0) {
                this->errorMsg = getTranslation(INSTALLDLG_ERROR_DIR_MISSING);
            } else if (!Fs::doesNativePathExist(this->locationBuffer)) {
                this->errorMsg = getTranslation(INSTALLDLG_ERROR_DIR_NOT_FOUND);
            }            
        }

        BoxedContainer* container = NULL;

        if (this->containerComboboxData.currentSelectedIndex!=0) {
            container = BoxedwineData::getContainers()[this->containerComboboxData.currentSelectedIndex-1];
        } else {            
            if (this->containerName[0]==0) {
                this->errorMsg = getTranslation(INSTALLDLG_ERROR_CONTAINER_NAME_MISSING);
            } else {
                std::string containerFilePath = GlobalSettings::getContainerFolder() + Fs::nativePathSeperator + this->containerName;
                if (Fs::doesNativePathExist(containerFilePath)) {
                    if (!Fs::isNativeDirectoryEmpty(containerFilePath)) {
                        this->errorMsgString = getTranslationWithFormat(INSTALLDLG_ERROR_CONTAINER_ALREADY_EXISTS, true,  containerFilePath.c_str());
                        this->errorMsg = this->errorMsgString.c_str();
                    }            
                } else if (!Fs::makeNativeDirs(containerFilePath)) {
                    this->errorMsgString = getTranslationWithFormat(INSTALLDLG_ERROR_FAILED_TO_CREATE_CONTAINER_DIR, true, strerror(errno));
                    this->errorMsg = this->errorMsgString.c_str();
                }   
            }
        }
        if (!this->errorMsg) {            
            GlobalSettings::startUpArgs = StartUpArgs(); // reset parameters
            if (!container) {
                std::string containerFilePath = GlobalSettings::getContainerFolder() + Fs::nativePathSeperator + this->containerName;
                container = BoxedContainer::createContainer(containerFilePath, this->containerName, GlobalSettings::getWineVersions()[wineVersionComboboxData.currentSelectedIndex].name);
            }
            container->launch(); // fill out startUpArgs specific to a container
            BoxedwineData::addContainer(container);
            if (this->runWineConfig) {
                GlobalSettings::startUpArgs.setRunWineConfigFirst(true);                        
                GlobalSettings::startUpArgs.readyToLaunch = true;
            }   

            if (this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_SETUP) {
                GlobalSettings::startUpArgs.setIsInstallingApp(true);
                GlobalSettings::startUpArgs.addArg(locationBuffer);
                GlobalSettings::startUpArgs.readyToLaunch = true;
            }
        }
    }

    if (this->errorMsg) {        
        if (!showMessageBox(buttonClicked, getTranslation(GENERIC_DLG_ERROR_TITLE), this->errorMsg)) {
            this->errorMsg = NULL;
        }
    }
    if (buttonClicked && !this->errorMsg) {          
        this->done();
    }
}