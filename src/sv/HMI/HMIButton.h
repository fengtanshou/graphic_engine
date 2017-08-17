#ifndef __HMI_BUTTON_H__
#define __HMI_BUTTON_H__

#include "../../XrCore/XrSrc/External/XrHeaders.h"
#include "../../XrCore/XrSrc/XrUILibrary/XrUILibrary.h"
#include "../DataStruct.h"

enum
{
	BUTTON_NORMAL = 0,
};//button error code

enum
{
    MAX_BUTTON_IMAGE_NUM = 30,
};

enum 
{
	BUTTON_HIDE = 0,
	BUTTON_SHOW = 1,
};

typedef enum SCREEN_TOUCH_INFO_TAG
{
    NO_COMMOND = 0x0,
    SCREEN_TOUCHED,
    SCREEN_HOLD,
    SCREEN_RELEASE,
}
SCREEN_TOUCH_INFO_T;

typedef enum Icon_Box_Type_Tag
{
	STATIC_ICON = 0,
	DYNAMIC_ICON,
}
Icon_Box_Type_T;

typedef struct Hmi_Button_Data_Tag
{
    float pos[2];
    float width;
    float height;
    int show_icon_num;  
    unsigned char icon_type; //0: static icon, 1: dynamic icon
    bool show_flag;
    char *icon_file_name[MAX_BUTTON_IMAGE_NUM];
	void (*delegate_func)();
}
Hmi_Button_Data_T;

typedef struct Hmi_Button_Slot_Tag
{	
	Hmi_Button_Data_T* buttonData;
	IMaterial *iconMtl;
}
Hmi_Button_Slot_T;

/* test :
	IUINode*  uiNode;	
	HMIButton* testButton;
	int uiNodeId = g_pIXrCore->CreateRenderNodeUI(RenderNodeType_UI2D, 0, &uiNode);
	Hmi_Button_Data_T testButtonData;
	testButtonData.pos[0] = 200;
    testButtonData.pos[1] = 200;
    testButtonData.width = icon_pgs_vp_lot_region_width;
    testButtonData.height = icon_pgs_vp_lot_region_height;
    testButtonData.show_flag = 1;
    testButtonData.icon_type = DYNAMIC_ICON;
    testButtonData.show_icon_num = 0;
    testButtonData.icon_file_name[0] = new char [50];

    sprintf(testButtonData.icon_file_name[0],"%sCar/pgs_parking_lot.dds",XR_RES);//

	testButton = new HMIButton(&testButtonData,uiNode);
*/

class HMIButton
{
public:
	HMIButton(Hmi_Button_Data_T* pButtonData,IUINode* uiNode);

	virtual ~HMIButton();

	virtual int Init();
	
	virtual int Update();

    virtual int SetShowIconNum(unsigned int index);

	virtual int SetX(float pos);
	virtual int SetY(float pos);
	virtual int SetWidth(float width);
	virtual int SetHeight(float height);

	int onClickListener(unsigned int pos_x, unsigned int pos_y, unsigned char action);

	int ButtonEffectClick();

	int GetButtonId();

	int SetVisibility(unsigned int flag);

	int ButtonShow();

	int ButtonHide();

private:
	Hmi_Button_Slot_T* m_buttonSlot;
	IUINode*		   m_uiNode;	
	int                m_uiNodeId;
	int                m_buttonId;
	int                m_buttonVisibleStatus;


};

#endif //#ifndef __HMI_BUTTON_H__