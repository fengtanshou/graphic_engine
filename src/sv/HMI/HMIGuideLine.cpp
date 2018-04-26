/*===========================================================================*\
 * FILE: HMIGuideLine.cpp
 *===========================================================================
 * Copyright 2003 O-Film Technologies, Inc., All Rights Reserved.
 * O-Film Confidential
 *
 * DESCRIPTION:
 *
 * ABBREVIATIONS:
 *   TODO: List of abbreviations used, or reference(s) to external document(s)
 *
 * TRACEABILITY INFO:
 *   Design Document(s):
 *     TODO: Update list of design document(s)
 *
 *   Requirements Document(s):
 *     TODO: Update list of requirements document(s)
 *
 *   Applicable Standards (in order of precedence: highest first):
 *
 * DEVIATIONS FROM STANDARDS:
 *   TODO: List of deviations from standards in this file, or
 *   None.
 *
\*===========================================================================*/
/*===========================================================================*\
 * Standard Header Files
\*===========================================================================*/
#include "HMIGuideLine.h"
#include "../AVMData.h"
#include "gpu_log.h"
#include "../SVScene.h"
//#include "global_init.h"
//extern float *gf_pgs_para;
extern SVScene* svscn;

#define SCALE_3D_TO_2D_X 0.34447


HMIGuideLine::HMIGuideLine(ISceneNode* pRootNode, HMIGuideLineDataT* pGuideLineData):m_guideLineVisibility(1)
{
    if(pRootNode == NULL
        || pGuideLineData == NULL
        || pGuideLineData->guideLineName == NULL
        || pGuideLineData->guideLineTexture[GUIDELINE_TEXTURE_NORMAL] == NULL)
    {
        return ;
    }

    m_rootNode = pRootNode;

    m_guideLineMtlId = m_rootNode->CreateMaterial(pGuideLineData->guideLineTextureType, &m_guideLineMtl);
    m_guideLineMtl->SetDiffuseMap(pGuideLineData->guideLineTexture[GUIDELINE_TEXTURE_NORMAL]);

    m_guideLineMeshId = m_rootNode->CreateMesh(ModelType_Plane_Dynamic, 2, pGuideLineData->guideLinePointNum, 0, pGuideLineData->guideLineName, &m_guideLineMesh);

    float pos[3] = {0.0};
    m_guideLineNodeId = m_rootNode->CreateModel(0, m_guideLineMtlId, -1, InsertFlag_Default,pos[0],pos[1],pos[2], 1.0, &m_guideLineNode);
    m_guideLineNode->SetMesh(m_guideLineMeshId);

    Int32 iSize;
    XRVertexLayout data_format;

    m_guideLineMesh->LockData(&m_vertext,&data_format,&iSize);

    Bev_3D_Param_T*  bev_3d_param;
    AVMData::GetInstance()->m_usc_data->GetBev3DParam(&bev_3d_param);
//    m_modelScale = 0.245;//bev_3d_param->model_param.model_scale;
//    m_modelScale = 1.0f / bev_3d_param->model_param.model_scale;
    m_modelScale = SCALE_3D_TO_2D_X;
//    Log_Debug("========================m_modelScale = %f==========",m_modelScale);
    m_modelBottom = -bev_3d_param->model_param.model_bottom * bev_3d_param->model_param.model_scale;

    m_calibCenterX = AVMData::GetInstance()->m_2D_lut->GetCalibReslt(POS_CALIB_CY);
    m_calibCenterY = AVMData::GetInstance()->m_2D_lut->GetCalibReslt(POS_CALIB_CX);
    m_calibMmppX = AVMData::GetInstance()->m_2D_lut->GetCalibReslt(POS_CALIB_PPMMX);
    m_calibMmppY = AVMData::GetInstance()->m_2D_lut->GetCalibReslt(POS_CALIB_PPMMY);

//    m_calibCenterX = gf_pgs_para[POS_CALIB_CY];
//    m_calibCenterY = gf_pgs_para[POS_CALIB_CX];
//    m_calibMmppX = gf_pgs_para[POS_CALIB_PPMMX];
//    m_calibMmppY = gf_pgs_para[POS_CALIB_PPMMY];

    m_vehParam = NULL;
    AVMData::GetInstance()->GetVehicleParam(&m_vehParam);

    m_bevRoiWidth = 216.0;
    m_bevRoiHeight = 480.0;

    m_guideLineData = new HMIGuideLineDataT;
    m_guideLineData->guideLineType = pGuideLineData->guideLineType ;
    m_guideLineData->guideLinePos = pGuideLineData->guideLinePos ;
    m_guideLineData->guideLineWidth = pGuideLineData->guideLineWidth ;
    m_guideLineData->guideLineLength = pGuideLineData->guideLineLength;
    m_guideLineData->guideLineSideDistanceFromVehicle = pGuideLineData->guideLineSideDistanceFromVehicle;
    m_guideLineData->guideLineStartDistanceFromVehicle = pGuideLineData->guideLineStartDistanceFromVehicle;
    m_guideLineData->guideLinePointNum = pGuideLineData->guideLinePointNum;
    m_guideLineData->guideLineVertexData = pGuideLineData->guideLineVertexData;

    Bev_DISPLAY_Param_Bev_Single_View_Roi_T* single_view_roi;


    AVMData::GetInstance()->m_usc_data->GetSingleViewRoi(&single_view_roi);

    m_frontCamZone[GUIDELINE_CAM_ZONE_LEFT] = AVMData::GetInstance()->GetFrontSingleViewRect(rect_left);//single_view_roi->front_single_view_roi_topleft_x;//0.0;
    m_frontCamZone[GUIDELINE_CAM_ZONE_TOP] = AVMData::GetInstance()->GetFrontSingleViewRect(rect_top);//single_view_roi->front_single_view_roi_topleft_y;//0.0;
    m_frontCamZone[GUIDELINE_CAM_ZONE_RIGHT] = AVMData::GetInstance()->GetFrontSingleViewRect(rect_right);//single_view_roi->front_single_view_roi_bottom_right_x;//1.0;
    m_frontCamZone[GUIDELINE_CAM_ZONE_BOTTOM] = AVMData::GetInstance()->GetFrontSingleViewRect(rect_bottom);//single_view_roi->front_single_view_roi_bottom_right_y;//1.0;
    for(int i = GUIDELINE_CAM_ZONE_LEFT;i<=GUIDELINE_CAM_ZONE_BOTTOM;i++)
    {
//        Log_Error("===========m_frontCamZone[%d]= %f =========",i,m_frontCamZone[i]);
    }
    m_rearCamZone[GUIDELINE_CAM_ZONE_LEFT] = AVMData::GetInstance()->GetRearSingleViewRect(rect_left);//single_view_roi->rear_single_view_roi_topleft_x;//0.81;
    m_rearCamZone[GUIDELINE_CAM_ZONE_TOP] = AVMData::GetInstance()->GetRearSingleViewRect(rect_top);//single_view_roi->rear_single_view_roi_topleft_y;//0.0;
    m_rearCamZone[GUIDELINE_CAM_ZONE_RIGHT] = AVMData::GetInstance()->GetRearSingleViewRect(rect_right);//single_view_roi->rear_single_view_roi_bottom_right_x;//0.19;
    m_rearCamZone[GUIDELINE_CAM_ZONE_BOTTOM] = AVMData::GetInstance()->GetRearSingleViewRect(rect_bottom);//single_view_roi->rear_single_view_roi_bottom_right_y;//1.0;
    for(int i = GUIDELINE_CAM_ZONE_LEFT;i<=GUIDELINE_CAM_ZONE_BOTTOM;i++)
    {
//        Log_Error("===========m_rearCamZone[%d]= %f =========",i,m_rearCamZone[i]);
    }
    m_leftCamZone[GUIDELINE_CAM_ZONE_LEFT] = AVMData::GetInstance()->GetLeftSingleViewRect(rect_left);//single_view_roi->left_single_view_roi_topleft_x;//0.0;
    m_leftCamZone[GUIDELINE_CAM_ZONE_TOP] = AVMData::GetInstance()->GetLeftSingleViewRect(rect_top);//single_view_roi->left_single_view_roi_topleft_y;//0.00;
    m_leftCamZone[GUIDELINE_CAM_ZONE_RIGHT] = AVMData::GetInstance()->GetLeftSingleViewRect(rect_right);//single_view_roi->left_single_view_roi_bottom_right_x;//1.0;
    m_leftCamZone[GUIDELINE_CAM_ZONE_BOTTOM] = AVMData::GetInstance()->GetLeftSingleViewRect(rect_bottom);//single_view_roi->left_single_view_roi_bottom_right_y;//1.0;

    for(int i = GUIDELINE_CAM_ZONE_LEFT;i<=GUIDELINE_CAM_ZONE_BOTTOM;i++)
    {
//        Log_Error("===========m_leftCamZone[%d]= %f =========",i,m_leftCamZone[i]);
    }

    m_rightCamZone[GUIDELINE_CAM_ZONE_LEFT] = AVMData::GetInstance()->GetRightSingleViewRect(rect_left);//single_view_roi->right_single_view_roi_topleft_x;//0.0;
    m_rightCamZone[GUIDELINE_CAM_ZONE_TOP] = AVMData::GetInstance()->GetRightSingleViewRect(rect_top);//single_view_roi->right_single_view_roi_topleft_y;//0.0;
    m_rightCamZone[GUIDELINE_CAM_ZONE_RIGHT] = AVMData::GetInstance()->GetRightSingleViewRect(rect_right);//single_view_roi->right_single_view_roi_bottom_x;//1.0;
    m_rightCamZone[GUIDELINE_CAM_ZONE_BOTTOM] = AVMData::GetInstance()->GetRightSingleViewRect(rect_bottom);//single_view_roi->right_single_view_roi_bottom_right_y;//1.0;
    for(int i = GUIDELINE_CAM_ZONE_LEFT;i<=GUIDELINE_CAM_ZONE_BOTTOM;i++)
    {
//        Log_Error("===========m_rightCamZone[%d]= %f =========",i,m_rightCamZone[i]);
    }
//    m_currentCamPos = rear_camera_index;
}

HMIGuideLine::~HMIGuideLine()
{
    SAFE_DELETE(m_guideLineData)
}

int HMIGuideLine::Update(float pSteeringWheel,int pDriveDirection)
{
    if(m_guideLineVisibility == 0)
        return GUIDELINE_NORMAL;

    float center[2];
    float radius[2];
    float theta[2];

    if(pSteeringWheel < 0.1 && pSteeringWheel > -0.1)
    {
        pSteeringWheel=0.1;
    }

    if(pDriveDirection == GUIDELINE_DIR_BACKWARD)
    {
        m_guideLineLength = m_guideLineData->guideLineLength;
        m_guideLineStartDistanceFromVehicle = m_guideLineData->guideLineStartDistanceFromVehicle;
        m_currentCamPos = rear_camera_index;
    }
    else if(pDriveDirection == GUIDELINE_DIR_FORWARD)
    {
        m_guideLineLength = - m_guideLineData->guideLineLength;
        m_guideLineStartDistanceFromVehicle = - m_guideLineData->guideLineStartDistanceFromVehicle;
//        m_guideLineData->guideLineLength *= -1;
//        m_guideLineData->guideLineStartDistanceFromVehicle *= -1;
        m_currentCamPos = front_camera_index;
    }
    else if(pDriveDirection == GUIDELINE_DIR_LEFT)
    {
        m_currentCamPos = left_camera_index;
    }
    else if(pDriveDirection == GUIDELINE_DIR_RIGHT)
    {
        m_currentCamPos = right_camera_index;
    }

    if(m_guideLineData->guideLineType == GUIDELINE_SINGLEVIEW_STATIC
        || m_guideLineData->guideLineType == GUIDELINE_DX3_LEFT_SINGLEVIEW_STATIC
        || m_guideLineData->guideLineType == GUIDELINE_DX3_RIGHT_SINGLEVIEW_STATIC)
    {
        SetStaticGuideLinePoints(pDriveDirection);
        GenerateStaticGuideLine(m_vertext);
    }
    else
    {
        CaculateCenter(&center[0],&center[1],radius,theta,pSteeringWheel,pDriveDirection);
        GenerateDynamicGuideLine(center[0],center[1],radius,theta, m_vertext);
    }

    m_guideLineMesh->UnLockData();

    return GUIDELINE_NORMAL;
}

int HMIGuideLine::SetStaticGuideLinePoints(int pDriveDirection)
{
    /*Log_Error("------m_vehParam->veh_width = %f",m_vehParam->veh_width);
    Log_Error("------m_vehParam->veh_rwheel2tail = %f",m_vehParam->veh_rwheel2tail);
    Log_Error("------m_vehParam->veh_length = %f",m_vehParam->veh_length);
    Log_Error("------m_vehParam->veh_fwheel2head = %f",m_vehParam->veh_fwheel2head);
    Log_Error("------m_vehParam->veh_axis_length = %f",m_vehParam->veh_axis_length);
    Log_Error("------m_vehParam->veh_tier_r = %f",m_vehParam->veh_tier_r);

    Log_Error("m_frontCamZone[GUIDELINE_CAM_ZONE_LEFT] = %f",m_frontCamZone[GUIDELINE_CAM_ZONE_LEFT]);
    Log_Error("m_frontCamZone[GUIDELINE_CAM_ZONE_TOP] = %f",m_frontCamZone[GUIDELINE_CAM_ZONE_TOP]);
    Log_Error("m_frontCamZone[GUIDELINE_CAM_ZONE_RIGHT] = %f",m_frontCamZone[GUIDELINE_CAM_ZONE_RIGHT]);
    Log_Error("m_frontCamZone[GUIDELINE_CAM_ZONE_BOTTOM] = %f",m_frontCamZone[GUIDELINE_CAM_ZONE_BOTTOM]);*/

    if(m_guideLineData->guideLineType == GUIDELINE_SINGLEVIEW_STATIC)
    {
        m_guideLineData->guideLineVertexData = new float[m_guideLineData->guideLinePointNum * 4];
        if(GUIDELINE_DIR_BACKWARD == pDriveDirection)
        {
            if(m_guideLineData->guideLinePos == GUIDELINE_POS_LEFT)
            {
                m_guideLineData->guideLineVertexData[0] = -m_vehParam->veh_width * 0.5
                    - m_guideLineData->guideLineSideDistanceFromVehicle;
                m_guideLineData->guideLineVertexData[2] = -m_vehParam->veh_width * 0.5
                    - m_guideLineData->guideLineSideDistanceFromVehicle + m_guideLineData->guideLineWidth;
                m_guideLineData->guideLineVertexData[4] = -m_vehParam->veh_width * 0.5
                    - m_guideLineData->guideLineSideDistanceFromVehicle;
                m_guideLineData->guideLineVertexData[6] = -m_vehParam->veh_width * 0.5
                    - m_guideLineData->guideLineSideDistanceFromVehicle + m_guideLineData->guideLineWidth;

                m_guideLineData->guideLineVertexData[1] = -m_vehParam->veh_rwheel2tail
                    - m_guideLineData->guideLineStartDistanceFromVehicle;
                m_guideLineData->guideLineVertexData[3] = -m_vehParam->veh_rwheel2tail
                    - m_guideLineData->guideLineStartDistanceFromVehicle;
                m_guideLineData->guideLineVertexData[5] = -m_vehParam->veh_rwheel2tail
                    - m_guideLineData->guideLineStartDistanceFromVehicle - m_guideLineData->guideLineLength;
                m_guideLineData->guideLineVertexData[7] = -m_vehParam->veh_rwheel2tail
                    - m_guideLineData->guideLineStartDistanceFromVehicle - m_guideLineData->guideLineLength;
            }
            if(m_guideLineData->guideLinePos == GUIDELINE_POS_RIGHT)
            {
                m_guideLineData->guideLineVertexData[0] = m_vehParam->veh_width * 0.5
                    + m_guideLineData->guideLineSideDistanceFromVehicle - m_guideLineData->guideLineWidth;
                m_guideLineData->guideLineVertexData[2] = m_vehParam->veh_width * 0.5
                    + m_guideLineData->guideLineSideDistanceFromVehicle;
                m_guideLineData->guideLineVertexData[4] = m_vehParam->veh_width * 0.5
                    + m_guideLineData->guideLineSideDistanceFromVehicle - m_guideLineData->guideLineWidth;
                m_guideLineData->guideLineVertexData[6] = m_vehParam->veh_width * 0.5
                    + m_guideLineData->guideLineSideDistanceFromVehicle;

                m_guideLineData->guideLineVertexData[1] = -m_vehParam->veh_rwheel2tail
                    - m_guideLineData->guideLineStartDistanceFromVehicle;
                m_guideLineData->guideLineVertexData[3] = -m_vehParam->veh_rwheel2tail
                    - m_guideLineData->guideLineStartDistanceFromVehicle;
                m_guideLineData->guideLineVertexData[5] = -m_vehParam->veh_rwheel2tail
                    - m_guideLineData->guideLineStartDistanceFromVehicle - m_guideLineData->guideLineLength;
                m_guideLineData->guideLineVertexData[7] = -m_vehParam->veh_rwheel2tail
                    - m_guideLineData->guideLineStartDistanceFromVehicle - m_guideLineData->guideLineLength;
            }
        }
        if(GUIDELINE_DIR_FORWARD == pDriveDirection)
        {
            if(m_guideLineData->guideLinePos == GUIDELINE_POS_LEFT)
            {
                m_guideLineData->guideLineVertexData[0] = -m_vehParam->veh_width * 0.5
                    - m_guideLineData->guideLineSideDistanceFromVehicle;
                m_guideLineData->guideLineVertexData[2] = -m_vehParam->veh_width * 0.5
                    - m_guideLineData->guideLineSideDistanceFromVehicle + m_guideLineData->guideLineWidth;
                m_guideLineData->guideLineVertexData[4] = -m_vehParam->veh_width * 0.5
                    - m_guideLineData->guideLineSideDistanceFromVehicle;
                m_guideLineData->guideLineVertexData[6] = -m_vehParam->veh_width * 0.5
                    - m_guideLineData->guideLineSideDistanceFromVehicle + m_guideLineData->guideLineWidth;

                m_guideLineData->guideLineVertexData[1] = m_vehParam->veh_fwheel2head + m_vehParam->veh_axis_length
                    + m_guideLineData->guideLineStartDistanceFromVehicle;
                m_guideLineData->guideLineVertexData[3] = m_vehParam->veh_fwheel2head + m_vehParam->veh_axis_length
                    + m_guideLineData->guideLineStartDistanceFromVehicle;
                m_guideLineData->guideLineVertexData[5] = m_vehParam->veh_fwheel2head + m_vehParam->veh_axis_length
                    + m_guideLineData->guideLineStartDistanceFromVehicle + m_guideLineData->guideLineLength;
                m_guideLineData->guideLineVertexData[7] = m_vehParam->veh_fwheel2head + m_vehParam->veh_axis_length
                    + m_guideLineData->guideLineStartDistanceFromVehicle + m_guideLineData->guideLineLength;
            }
            if(m_guideLineData->guideLinePos == GUIDELINE_POS_RIGHT)
            {
                m_guideLineData->guideLineVertexData[0] = m_vehParam->veh_width * 0.5
                    + m_guideLineData->guideLineSideDistanceFromVehicle - m_guideLineData->guideLineWidth;
                m_guideLineData->guideLineVertexData[2] = m_vehParam->veh_width * 0.5
                    + m_guideLineData->guideLineSideDistanceFromVehicle;
                m_guideLineData->guideLineVertexData[4] = m_vehParam->veh_width * 0.5
                    + m_guideLineData->guideLineSideDistanceFromVehicle - m_guideLineData->guideLineWidth;
                m_guideLineData->guideLineVertexData[6] = m_vehParam->veh_width * 0.5
                    + m_guideLineData->guideLineSideDistanceFromVehicle;

                m_guideLineData->guideLineVertexData[1] = m_vehParam->veh_fwheel2head + m_vehParam->veh_axis_length
                    + m_guideLineData->guideLineStartDistanceFromVehicle;
                m_guideLineData->guideLineVertexData[3] = m_vehParam->veh_fwheel2head + m_vehParam->veh_axis_length
                    + m_guideLineData->guideLineStartDistanceFromVehicle;
                m_guideLineData->guideLineVertexData[5] = m_vehParam->veh_fwheel2head + m_vehParam->veh_axis_length
                    + m_guideLineData->guideLineStartDistanceFromVehicle + m_guideLineData->guideLineLength;
                m_guideLineData->guideLineVertexData[7] = m_vehParam->veh_fwheel2head + m_vehParam->veh_axis_length
                    + m_guideLineData->guideLineStartDistanceFromVehicle + m_guideLineData->guideLineLength;
            }
        }
    }
}

int HMIGuideLine::GenerateStaticGuideLine(float* pVertex)
{
    int Loop = 0;
    int slotid = 0;
    float world_coord[2];
    float model_coord[2];
    float texture[m_guideLineData->guideLinePointNum * 2][2];
    for(Loop = 0; Loop < m_guideLineData->guideLinePointNum*2; Loop++)
    {
        world_coord[0] = m_guideLineData->guideLineVertexData[Loop*2 + 0];
        world_coord[1] = m_guideLineData->guideLineVertexData[Loop*2 + 1];

        CalWorld2ModelCoordinate(model_coord,world_coord,&(texture[Loop][0]));
//        Log_Debug("========%d x= %f  y= %f=====",Loop,texture[Loop][0],texture[Loop][1]);
        pVertex[slotid+0] = model_coord[0];
        pVertex[slotid+1] = m_modelBottom;
        pVertex[slotid+2] = model_coord[1];

        if(m_guideLineData->guideLineType == GUIDELINE_DX3_LEFT_SINGLEVIEW_STATIC)
        {
            pVertex[slotid+0] = -model_coord[1];
            pVertex[slotid+1] = model_coord[0];
            pVertex[slotid+2] = model_coord[0];
//            Log_Debug("%d=%f,%d=%f,%d=%f",slotid+0,pVertex[slotid+0],slotid+1,pVertex[slotid+1],slotid+2,pVertex[slotid+2]);
        }
        else if(m_guideLineData->guideLineType == GUIDELINE_DX3_RIGHT_SINGLEVIEW_STATIC)
        {
            pVertex[slotid+0] = model_coord[1];
            pVertex[slotid+1] = -model_coord[0];
            pVertex[slotid+2] = -model_coord[0];
//            Log_Debug("%d=%f,%d=%f,%d=%f",slotid+0,pVertex[slotid+0],slotid+1,pVertex[slotid+1],slotid+2,pVertex[slotid+2]);
        }
        else if(m_guideLineData->guideLineType == GUIDELINE_SINGLEVIEW_STATIC)
        {
            pVertex[slotid+0] = model_coord[0];
            pVertex[slotid+1] = model_coord[1];
            pVertex[slotid+2] = model_coord[1];
//            Log_Debug("%d=%f,%d=%f,%d=%f",slotid+0,pVertex[slotid+0],slotid+1,pVertex[slotid+1],slotid+2,pVertex[slotid+2]);
        }
        else if(m_guideLineData->guideLineType == GUIDELINE_BEV_STATIC)
        {
            pVertex[slotid+1] = pVertex[slotid+2];
//            Log_Debug("%d=%f,%d=%f,%d=%f",slotid+0,pVertex[slotid+0],slotid+1,pVertex[slotid+1],slotid+2,pVertex[slotid+2]);
        }

        if(m_guideLineData->guideLineType == GUIDELINE_3D_STATIC)
        {

        }

        slotid+=8;

    }
    if(m_guideLineData->guideLineType == GUIDELINE_DX3_LEFT_SINGLEVIEW_STATIC)
    {
        float texturey = 0;
        float textureXmin = 0;
        float textureXmax = 1;
        for(Loop = 0; Loop < m_guideLineData->guideLinePointNum*2; Loop++)
        {
            texturey += texture[Loop][1];
        }
        texturey = texturey / Loop;
        if(texturey >= 0.5)
        {
            m_leftCamZone[GUIDELINE_CAM_ZONE_BOTTOM] = 1.0;
            m_leftCamZone[GUIDELINE_CAM_ZONE_TOP] = 2.0 * texturey - 1.0 ;
        }
        else
        {
            m_leftCamZone[GUIDELINE_CAM_ZONE_TOP] = 0.0;
            m_leftCamZone[GUIDELINE_CAM_ZONE_BOTTOM] = 2 * texturey;
        }
        if(m_guideLineData->guideLinePointNum == 2)
        {
            textureXmin = (texture[0][0] + texture[1][0]) / 2.0;
            textureXmax = (texture[2][0] + texture[3][0]) / 2.0;
            if(textureXmin > textureXmax)
            {
                float tmpX = textureXmin;
                textureXmin = textureXmax;
                textureXmax = tmpX;
            }
            m_leftCamZone[GUIDELINE_CAM_ZONE_LEFT] = textureXmin - (textureXmax - textureXmin) / 5.0;
            m_leftCamZone[GUIDELINE_CAM_ZONE_RIGHT] = textureXmax + (textureXmax - textureXmin) / 5.0;
            if(m_leftCamZone[GUIDELINE_CAM_ZONE_LEFT] < 0) m_leftCamZone[GUIDELINE_CAM_ZONE_LEFT] = 0;
            if( m_leftCamZone[GUIDELINE_CAM_ZONE_RIGHT] > 1)  m_leftCamZone[GUIDELINE_CAM_ZONE_RIGHT] = 1;
        }
//        float* tmpSingleViewVertex;
//        if(svscn -> GetSingleviewVertex(LEFT_SINGLE_VIEW) != NULL)
//        {
//            tmpSingleViewVertex = svscn -> GetSingleviewVertex(LEFT_SINGLE_VIEW);
//            tmpSingleViewVertex[3 + 14] = m_leftCamZone[GUIDELINE_CAM_ZONE_LEFT];
//            tmpSingleViewVertex[4 + 14] = m_leftCamZone[GUIDELINE_CAM_ZONE_TOP];
//            tmpSingleViewVertex[3 + 0] = m_leftCamZone[GUIDELINE_CAM_ZONE_RIGHT];
//            tmpSingleViewVertex[4 + 0] = m_leftCamZone[GUIDELINE_CAM_ZONE_TOP];
//            tmpSingleViewVertex[3 + 21] = m_leftCamZone[GUIDELINE_CAM_ZONE_LEFT];
//            tmpSingleViewVertex[4 + 21] = m_leftCamZone[GUIDELINE_CAM_ZONE_BOTTOM];
//            tmpSingleViewVertex[3 + 7] = m_leftCamZone[GUIDELINE_CAM_ZONE_RIGHT];
//            tmpSingleViewVertex[4 + 7] = m_leftCamZone[GUIDELINE_CAM_ZONE_BOTTOM];
//            if(svscn -> GetSingleViewMesh(LEFT_SINGLE_VIEW) != NULL)
//            {
//                (svscn -> GetSingleViewMesh(LEFT_SINGLE_VIEW))->UnLockData();
//            }
//        }
    }
    else if(m_guideLineData->guideLineType == GUIDELINE_DX3_RIGHT_SINGLEVIEW_STATIC)
    {
        float texturey = 0;
        float textureXmin = 0;
        float textureXmax = 1;
        for(Loop = 0; Loop < m_guideLineData->guideLinePointNum*2; Loop++)
        {
            texturey += texture[Loop][1];
        }
        texturey = texturey / Loop;
        if(texturey >= 0.5)
        {
            m_rightCamZone[GUIDELINE_CAM_ZONE_BOTTOM] = 1.0;
            m_rightCamZone[GUIDELINE_CAM_ZONE_TOP] = 2.0 * texturey - 1.0 ;
        }
        else
        {
            m_rightCamZone[GUIDELINE_CAM_ZONE_TOP] = 0.0;
            m_rightCamZone[GUIDELINE_CAM_ZONE_BOTTOM] = 2 * texturey;
        }
        if(m_guideLineData->guideLinePointNum == 2)
        {
            textureXmin = (texture[0][0] + texture[1][0]) / 2.0;
            textureXmax = (texture[2][0] + texture[3][0]) / 2.0;
            if(textureXmin > textureXmax)
            {
                float tmpX = textureXmin;
                textureXmin = textureXmax;
                textureXmax = tmpX;
            }
            m_rightCamZone[GUIDELINE_CAM_ZONE_LEFT] = textureXmin - (textureXmax - textureXmin) / 5.0;
            m_rightCamZone[GUIDELINE_CAM_ZONE_RIGHT] = textureXmax + (textureXmax - textureXmin) / 5.0;
            if(m_rightCamZone[GUIDELINE_CAM_ZONE_LEFT] < 0) m_rightCamZone[GUIDELINE_CAM_ZONE_LEFT] = 0;
            if( m_rightCamZone[GUIDELINE_CAM_ZONE_RIGHT] > 1)  m_rightCamZone[GUIDELINE_CAM_ZONE_RIGHT] = 1;
        }
//        float *tmpSingleViewVertex;
//        if (svscn->GetSingleviewVertex(RIGHT_SINGLE_VIEW) != NULL)
//        {
//            tmpSingleViewVertex = svscn->GetSingleviewVertex(RIGHT_SINGLE_VIEW);
//            tmpSingleViewVertex[3 + 7] = m_rightCamZone[GUIDELINE_CAM_ZONE_LEFT];
//            tmpSingleViewVertex[4 + 7] = m_rightCamZone[GUIDELINE_CAM_ZONE_TOP];
//            tmpSingleViewVertex[3 + 21] = m_rightCamZone[GUIDELINE_CAM_ZONE_RIGHT];
//            tmpSingleViewVertex[4 + 21] = m_rightCamZone[GUIDELINE_CAM_ZONE_TOP];
//            tmpSingleViewVertex[3 + 0] = m_rightCamZone[GUIDELINE_CAM_ZONE_LEFT];
//            tmpSingleViewVertex[4 + 0] = m_rightCamZone[GUIDELINE_CAM_ZONE_BOTTOM];
//            tmpSingleViewVertex[3 + 14] = m_rightCamZone[GUIDELINE_CAM_ZONE_RIGHT];
//            tmpSingleViewVertex[4 + 14] = m_rightCamZone[GUIDELINE_CAM_ZONE_BOTTOM];
//            if (svscn->GetSingleViewMesh(RIGHT_SINGLE_VIEW) != NULL)
//            {
//                (svscn->GetSingleViewMesh(RIGHT_SINGLE_VIEW))->UnLockData();
//            }
//        }
    }
    return GUIDELINE_NORMAL;
}
int HMIGuideLine::GenerateDynamicGuideLine(float pCenterX, float pCenterY,float pRadius[],float pStartAngle[],float*pVertex)
{
    float AngleStart[2];
    float AngleEnd;
    float xLeft;
    float yLeft;
    float xRight;
    float yRight;
    float xNormal;
    float yNormal;
    float zNormal;
    float temp =0.0;
    float step;
    float tempx;
    float tempy;
    unsigned int Loop;
    unsigned int index=0,slotid=0;
    float flastx=0,flasty=0;

    float totalAngle;
    float guideLineStartAngleFromVehicle[2];
    float startAngle[2];

    float world_coord[2];
    float model_coord[2];
    float output[3];
    float tempCam[3];

    for(int pos = 0; pos < 2; pos++) //引导线左右两侧，左侧时pCenterX < 0, 右侧时pCenterX > 0
    {
        totalAngle = m_guideLineLength / pRadius[pos];

        if(pCenterX < 0)
        {
            guideLineStartAngleFromVehicle[pos] = -m_guideLineStartDistanceFromVehicle / pRadius[pos];
            step = -(totalAngle + guideLineStartAngleFromVehicle[pos]) / m_guideLineData->guideLinePointNum;
            AngleStart[pos] = pStartAngle[pos];
        }
        else
        {
            guideLineStartAngleFromVehicle[pos] = m_guideLineStartDistanceFromVehicle / pRadius[pos];
            step = (totalAngle - guideLineStartAngleFromVehicle[pos]) / m_guideLineData->guideLinePointNum;
            AngleStart[pos] = -GUIDELINE_PI + pStartAngle[pos];
        }
    }

    float texture[m_guideLineData->guideLinePointNum * 2][2];

    for(Loop = 0; Loop < m_guideLineData->guideLinePointNum * 2; Loop++)
    {
        index = Loop%2;
        xLeft= pCenterX+(pRadius[index])*cos(AngleStart[index] + guideLineStartAngleFromVehicle[index] +(Loop/2)*step);
        yLeft= pCenterY+(pRadius[index])*sin(AngleStart[index] + guideLineStartAngleFromVehicle[index]+(Loop/2)*step);

        world_coord[0] = xLeft;
        world_coord[1] = yLeft;

        CalWorld2ModelCoordinate(model_coord,world_coord,&(texture[Loop][0]));
//        if(Loop == 0)
//        {
//            Log_Error("xLeft = %f, yLeft = %f",xLeft,yLeft);
//            Log_Error("========x= %f  y= %f=====",texture[Loop][0],texture[Loop][1]);
//            Log_Error("========x= %f  y= %f=====",model_coord[0],model_coord[1]);
//        }
        pVertex[slotid+0] = model_coord[0];
        pVertex[slotid+1] = m_modelBottom;
        pVertex[slotid+2] = model_coord[1];

        if(m_guideLineData->guideLineType == GUIDELINE_SINGLEVIEW_DYNAMIC)
        {
            pVertex[slotid+0] = model_coord[0];
            pVertex[slotid+1] = model_coord[1];
            pVertex[slotid+2] = model_coord[1];
        }
        else if(m_guideLineData->guideLineType == GUIDELINE_BEV_DYNAMIC)
        {
            pVertex[slotid+1] = pVertex[slotid+2];
        }

        if(m_guideLineData->guideLineType == GUIDELINE_3D_DYNAMIC)
        {

        }

        slotid+=8;

    }


    return GUIDELINE_NORMAL;
}
int HMIGuideLine::CaculateCenter(float* pCenterX, float* pCenterY,
                                    float pRadius[], float* pTheta, float pSteeringWheel, int pDirect)
{
    float fTurnRadius;
    float fBackWheel2Tail = m_vehParam->veh_rwheel2tail;//AVMData::GetInstance()->m_p_can_data->Get_Vehicle_Rear_Wheel2Bumper_Length();
    float fVehicleWidth = m_vehParam->veh_width;//AVMData::GetInstance()->m_p_can_data->Get_Vehicle_Width();
    float fFrontWheel2Bumper = m_vehParam->veh_fwheel2head;//AVMData::GetInstance()->m_p_can_data->Get_Vehicle_Front_Wheel2Bumper_Length();
    float fAxisLength = m_vehParam->veh_axis_length + fFrontWheel2Bumper;//AVMData::GetInstance()->m_p_can_data->Get_Vehicle_Axis_Length() + fFrontWheel2Bumper;

//    Log_Error("veh_axis_length = %f , fFrontWheel2Bumper = %f , fAxisLength = %f ",m_vehParam->veh_axis_length,fFrontWheel2Bumper,fAxisLength);

    CalSteeringWheel2Radius(&fTurnRadius,pSteeringWheel,pDirect);

    if(m_guideLineData->guideLineType == GUIDELINE_BEV_ASSIST_DYNAMIC)
    {
        fBackWheel2Tail = 0.0; //无需使用后轮至保险杠的距离计算引导线半径，引导线半径直接等于计算的转弯半径+引导线宽度
        fAxisLength = 0.0; //引导线半径直接等于计算的转弯半径+引导线宽度
    }

    float turnRadius = 0.0;
    *pCenterY = 0.0;
    if(GUIDELINE_DIR_BACKWARD == pDirect)
    {
       if(fTurnRadius < 0.0)
       {
           if(m_guideLineData->guideLinePos == GUIDELINE_POS_LEFT)
           {
                turnRadius = fTurnRadius + m_guideLineData->guideLineSideDistanceFromVehicle;
                pRadius[0] = sqrt(turnRadius * turnRadius + fBackWheel2Tail * fBackWheel2Tail);
                pRadius[1] = sqrt((-turnRadius + m_guideLineData->guideLineWidth) * (-turnRadius + m_guideLineData->guideLineWidth) + fBackWheel2Tail * fBackWheel2Tail);
                pTheta[0] = atan(fBackWheel2Tail / turnRadius);
                pTheta[1] = atan(fBackWheel2Tail / (turnRadius - m_guideLineData->guideLineWidth));
           }
           else if(m_guideLineData->guideLinePos == GUIDELINE_POS_RIGHT)
           {
                turnRadius = fTurnRadius - m_guideLineData->guideLineSideDistanceFromVehicle;
                pRadius[1] = sqrt((-turnRadius+ fVehicleWidth)*(-turnRadius+ fVehicleWidth)+fBackWheel2Tail*fBackWheel2Tail);
                pRadius[0] = sqrt((-turnRadius + fVehicleWidth - m_guideLineData->guideLineWidth)*(-turnRadius + fVehicleWidth - m_guideLineData->guideLineWidth) + fBackWheel2Tail * fBackWheel2Tail);
                pTheta[1] = atan(fBackWheel2Tail / (turnRadius - fVehicleWidth));
                pTheta[0] = atan(fBackWheel2Tail / (turnRadius - fVehicleWidth + m_guideLineData->guideLineWidth));
           }

           *pCenterX = fTurnRadius - fVehicleWidth/2.0;

        }
        else
        {
            if(m_guideLineData->guideLinePos == GUIDELINE_POS_LEFT)
            {
                turnRadius = fTurnRadius + m_guideLineData->guideLineSideDistanceFromVehicle;
                pRadius[1] = sqrt((turnRadius+fVehicleWidth - m_guideLineData->guideLineWidth)*(turnRadius+fVehicleWidth - m_guideLineData->guideLineWidth)+fBackWheel2Tail*fBackWheel2Tail);
                pRadius[0] = sqrt((turnRadius + fVehicleWidth)*(turnRadius + fVehicleWidth)+fBackWheel2Tail*fBackWheel2Tail);
                pTheta[1]=atan(fBackWheel2Tail/(turnRadius+fVehicleWidth - m_guideLineData->guideLineWidth));
                pTheta[0]=atan(fBackWheel2Tail/(turnRadius + fVehicleWidth));
            }
            else
            {
                turnRadius = fTurnRadius - m_guideLineData->guideLineSideDistanceFromVehicle;
                pRadius[1] = sqrt(turnRadius*turnRadius+fBackWheel2Tail*fBackWheel2Tail);
                pRadius[0] = sqrt((turnRadius + m_guideLineData->guideLineWidth)*(turnRadius + m_guideLineData->guideLineWidth)+fBackWheel2Tail*fBackWheel2Tail);
                pTheta[1]=atan(fBackWheel2Tail/turnRadius);
                pTheta[0]=atan(fBackWheel2Tail/(turnRadius + m_guideLineData->guideLineWidth));
            }

            *pCenterX = fTurnRadius + fVehicleWidth/2.0;
        }
   }
   else if(GUIDELINE_DIR_FORWARD == pDirect)  //foward using rear wheel inside and front head outside
   {
        if(fTurnRadius < 0)
        {
            if(m_guideLineData->guideLinePos == GUIDELINE_POS_LEFT)
            {
                turnRadius = fTurnRadius + m_guideLineData->guideLineSideDistanceFromVehicle;
                pRadius[0] = sqrt((-turnRadius)*(-turnRadius)+fAxisLength*fAxisLength);
                pRadius[1] = sqrt((-turnRadius + m_guideLineData->guideLineWidth)*(-turnRadius + m_guideLineData->guideLineWidth)+fAxisLength*fAxisLength);
                pTheta[0]=-atan(fAxisLength/(turnRadius));
                pTheta[1]=-atan(fAxisLength/(turnRadius-m_guideLineData->guideLineWidth));
            }
            else
            {
                turnRadius = fTurnRadius - m_guideLineData->guideLineSideDistanceFromVehicle;
                pRadius[0] = sqrt((-turnRadius + fVehicleWidth - m_guideLineData->guideLineWidth)*(-turnRadius + fVehicleWidth - m_guideLineData->guideLineWidth)+fAxisLength*fAxisLength);
                pRadius[1] = sqrt((-turnRadius+fVehicleWidth)*(-turnRadius+fVehicleWidth)+fAxisLength*fAxisLength);
                pTheta[0]=-atan(fAxisLength/(turnRadius - fVehicleWidth + m_guideLineData->guideLineWidth));
                pTheta[1]=-atan(fAxisLength/(turnRadius - fVehicleWidth));
            }

           *pCenterX = fTurnRadius- fVehicleWidth/2;

        }
        else
        {
            if(m_guideLineData->guideLinePos == GUIDELINE_POS_LEFT)
            {
                turnRadius = fTurnRadius + m_guideLineData->guideLineSideDistanceFromVehicle;
                pRadius[1] = sqrt((turnRadius+fVehicleWidth - m_guideLineData->guideLineWidth)*(turnRadius+fVehicleWidth - m_guideLineData->guideLineWidth) + fAxisLength*fAxisLength);
                pRadius[0] = sqrt((turnRadius+fVehicleWidth)*(turnRadius+fVehicleWidth)+fAxisLength*fAxisLength);
                pTheta[1]=-atan(fAxisLength/(turnRadius + fVehicleWidth - m_guideLineData->guideLineWidth));
                pTheta[0]=-atan(fAxisLength/(turnRadius + fVehicleWidth));
            }
            else
            {
                turnRadius = fTurnRadius - m_guideLineData->guideLineSideDistanceFromVehicle;
                pRadius[1] = sqrt((turnRadius)*(turnRadius)+fAxisLength*fAxisLength);
                pRadius[0] = sqrt((turnRadius + m_guideLineData->guideLineWidth)*(turnRadius + m_guideLineData->guideLineWidth)+fAxisLength*fAxisLength);
                pTheta[1]=-atan(fAxisLength/(turnRadius));
                pTheta[0]=-atan(fAxisLength/(turnRadius + m_guideLineData->guideLineWidth));
            }

            *pCenterX = fTurnRadius + fVehicleWidth/2;

        }

   }
    return GUIDELINE_NORMAL;
}

int HMIGuideLine::CalSteeringWheel2Radius(float *pRadius,float pSteerAngle,int pGearState)
{
   //get value from config-file
    float steerCountAtLock = 0;//pVhiclePara->fSteeringWheelAtlock; // steering wheel sensor
    float turnRadiusAtLock = 0;// pVhiclePara->fRmin; // meters minimum truning radius
    float curvatureAtLock = 0; // radians = 1/radius

    float VEHICLE_WHEELBASE = m_vehParam->veh_axis_length;
    //float steerAngle = Get_Vehicle_Real_Steer_Angle();
    float vehCurv = 0;
    float temp = vehCurv;
    float c_in =vehCurv;
    float max_beta = 0;
    float beta = 0;
    float c = 0;
    if(pGearState == GUIDELINE_DIR_BACKWARD)
    {
        if(pSteerAngle>0)
        {
            steerCountAtLock = m_vehParam->vehicle_max_steering_wheel_angle_leftturn;// AVMData::GetInstance()->m_p_can_data->Get_Vehicle_Max_Deg_Steering_Wheel_Left_Turn();
            turnRadiusAtLock =  m_vehParam->vehicle_min_radius_reverse_leftturn_rear_wheel_center;// AVMData::GetInstance()->m_p_can_data->Get_Vehicle_Min_Radius_Gear_R_Left_Turn_Rear_Axis_Cent();//pVhiclePara->afRminArray[REAR_LEFT_RADIUS];
        }
        else
        {
            steerCountAtLock = m_vehParam->vehicle_max_steering_wheel_angle_rightturn;//AVMData::GetInstance()->m_p_can_data->Get_Vehicle_Max_Deg_Steering_Wheel_Right_Turn();//pVhiclePara->afSteeringWheelAtlock[REAR_RIGHT_RADIUS];
            turnRadiusAtLock = m_vehParam->vehicle_min_radius_reverse_rightturn_rear_wheel_center; //AVMData::GetInstance()->m_p_can_data->Get_Vehicle_Min_Radius_Gear_R_Right_Turn_Rear_Axis_Cent();//pVhiclePara->afRminArray[REAR_RIGHT_RADIUS];
        }
    }
    else
    {
        if(pSteerAngle>0)
        {
            steerCountAtLock = m_vehParam->vehicle_max_steering_wheel_angle_leftturn;// AVMData::GetInstance()->m_p_can_data->Get_Vehicle_Max_Deg_Steering_Wheel_Left_Turn();
            turnRadiusAtLock = m_vehParam->vehicle_min_radius_forward_leftturn_rear_wheel_center;// AVMData::GetInstance()->m_p_can_data->Get_Vehicle_Min_Radius_Gear_R_Left_Turn_Rear_Axis_Cent();//pVhiclePara->afRminArray[REAR_LEFT_RADIUS];
        }
        else
        {
            steerCountAtLock = m_vehParam->vehicle_max_steering_wheel_angle_rightturn;//AVMData::GetInstance()->m_p_can_data->Get_Vehicle_Max_Deg_Steering_Wheel_Right_Turn();//pVhiclePara->afSteeringWheelAtlock[REAR_RIGHT_RADIUS];
            turnRadiusAtLock = m_vehParam->vehicle_min_radius_forward_rightturn_rear_wheel_center;
        }
    }

    curvatureAtLock = 1.0f/turnRadiusAtLock; // radians = 1/radius
    vehCurv = ((float)pSteerAngle / steerCountAtLock) * curvatureAtLock;
    temp = vehCurv;
    c_in =vehCurv;
    //float VEHICLE_WHEELBASE = 2.7f; //2.85f
    max_beta = atan(VEHICLE_WHEELBASE / turnRadiusAtLock);
    beta = max_beta * c_in / curvatureAtLock;
    c = tan(beta) / VEHICLE_WHEELBASE;

    vehCurv = c ;

    temp =    (vehCurv>curvatureAtLock? curvatureAtLock:(vehCurv<-curvatureAtLock ? -curvatureAtLock:vehCurv));
    *pRadius=1/temp;

    return GUIDELINE_NORMAL;
}
int HMIGuideLine::CalWorld2ModelCoordinate(float *out_Model_Coord,float *in_world_coord,float *out_texture)
{
    if(m_guideLineData->guideLineType == GUIDELINE_3D_DYNAMIC)
    {
        out_Model_Coord[1] = -m_modelScale*(in_world_coord[1]-((m_vehParam->veh_length/2.0) - m_vehParam->veh_rwheel2tail));
        out_Model_Coord[0] = m_modelScale*(in_world_coord[0]);
    }
    else if(m_guideLineData->guideLineType == GUIDELINE_SINGLEVIEW_DYNAMIC
            || m_guideLineData->guideLineType == GUIDELINE_SINGLEVIEW_STATIC
            || m_guideLineData->guideLineType == GUIDELINE_DX3_LEFT_SINGLEVIEW_STATIC
            || m_guideLineData->guideLineType == GUIDELINE_DX3_RIGHT_SINGLEVIEW_STATIC)
    {
        float texture[2];
        float inWorldCoord[3];

        inWorldCoord[0] = m_modelScale*(in_world_coord[0]);
        inWorldCoord[1] = m_modelBottom;
        inWorldCoord[2] = -m_modelScale*(in_world_coord[1]-((m_vehParam->veh_length/2.0) - m_vehParam->veh_rwheel2tail));

        if(m_currentCamPos == rear_camera_index)
        {
            inWorldCoord[0] = -inWorldCoord[0];
            inWorldCoord[2] = -inWorldCoord[2];
        }
        else if(m_currentCamPos == left_camera_index)
        {
            inWorldCoord[0] = -inWorldCoord[0];
            inWorldCoord[2] = -inWorldCoord[2];
        }
        AVMData::GetInstance()->CalcUVTextureSV(inWorldCoord,texture,m_currentCamPos);
        out_texture[0] = texture[0];
        out_texture[1] = texture[1];
//        Log_Debug("texture[0]=%f,texture[1]=%f",texture[0],texture[1]);

        if(m_currentCamPos == rear_camera_index)
        {
            //out_Model_Coord[2] = 0.0;

            out_Model_Coord[1] = -(texture[1]-m_rearCamZone[GUIDELINE_CAM_ZONE_TOP])/(m_rearCamZone[GUIDELINE_CAM_ZONE_BOTTOM]-m_rearCamZone[GUIDELINE_CAM_ZONE_TOP])/0.5+1;
            out_Model_Coord[0] = (-texture[0]+m_rearCamZone[GUIDELINE_CAM_ZONE_LEFT])/(m_rearCamZone[GUIDELINE_CAM_ZONE_LEFT]-m_rearCamZone[GUIDELINE_CAM_ZONE_RIGHT])/0.5-1;
            //out_Model_Coord[0] =-out_Model_Coord[0];

        }
        else if(m_currentCamPos == front_camera_index)
        {
            //out_Model_Coord[2] = 0.0;
            out_Model_Coord[1] = -(texture[1]-m_frontCamZone[GUIDELINE_CAM_ZONE_TOP])/(m_frontCamZone[GUIDELINE_CAM_ZONE_BOTTOM]-m_frontCamZone[GUIDELINE_CAM_ZONE_TOP])/0.5+1;
            out_Model_Coord[0] = (texture[0]-m_frontCamZone[GUIDELINE_CAM_ZONE_LEFT])/(m_frontCamZone[GUIDELINE_CAM_ZONE_RIGHT]-m_frontCamZone[GUIDELINE_CAM_ZONE_LEFT])/0.5-1;

        }
        else if(m_currentCamPos == left_camera_index)
        {
            //out_Model_Coord[2] = 0.0;
            out_Model_Coord[1] = -(texture[1]-m_leftCamZone[GUIDELINE_CAM_ZONE_TOP])/(m_leftCamZone[GUIDELINE_CAM_ZONE_BOTTOM]-m_leftCamZone[GUIDELINE_CAM_ZONE_TOP])/0.5+1;
            out_Model_Coord[0] = (texture[0]-m_leftCamZone[GUIDELINE_CAM_ZONE_LEFT])/(m_leftCamZone[GUIDELINE_CAM_ZONE_RIGHT]-m_leftCamZone[GUIDELINE_CAM_ZONE_LEFT])/0.5-1;

        }
        else if(m_currentCamPos == right_camera_index)
        {
//            Log_Debug("texture[0]=%f,texture[1]=%f",texture[0],texture[1]);
            //out_Model_Coord[2] = 0.0;
            out_Model_Coord[1] = -(texture[1]-m_rightCamZone[GUIDELINE_CAM_ZONE_TOP])/(m_rightCamZone[GUIDELINE_CAM_ZONE_BOTTOM]-m_rightCamZone[GUIDELINE_CAM_ZONE_TOP])/0.5+1;
            out_Model_Coord[0] = (texture[0]-m_rightCamZone[GUIDELINE_CAM_ZONE_LEFT])/(m_rightCamZone[GUIDELINE_CAM_ZONE_RIGHT]-m_rightCamZone[GUIDELINE_CAM_ZONE_LEFT])/0.5-1;

//            Log_Debug("out_Model_Coord[0]=%f,out_Model_Coord[1]=%f",out_Model_Coord[0],out_Model_Coord[1]);
        }
    }
    else if(m_guideLineData->guideLineType == GUIDELINE_BEV_DYNAMIC)
    {
       out_Model_Coord[1] = 1.0-(((in_world_coord[1])/(0.0 - m_calibMmppY) + m_calibCenterY)/(m_bevRoiHeight/2.0));
       out_Model_Coord[0] =((in_world_coord[0])/m_calibMmppX + m_calibCenterX - (m_bevRoiWidth/2.0))/(m_bevRoiWidth/2.0);
    }
//    Log_Debug("out_Model_Coord[0]=%f,out_Model_Coord[1]=%f",out_Model_Coord[0],out_Model_Coord[1]);
    return GUIDELINE_NORMAL;
}

int HMIGuideLine::ResetCamZone(unsigned char pCamPos, float* pCamZone)
{
    if(m_guideLineData->guideLineType == GUIDELINE_SINGLEVIEW_DYNAMIC
        || m_guideLineData->guideLineType == GUIDELINE_SINGLEVIEW_STATIC
        || m_guideLineData->guideLineType == GUIDELINE_DX3_LEFT_SINGLEVIEW_STATIC
        || m_guideLineData->guideLineType == GUIDELINE_DX3_RIGHT_SINGLEVIEW_STATIC)
    {
        if(pCamPos == front_camera_index)
        {
           memcpy(m_frontCamZone,pCamZone,4*sizeof(float));
        }
        else if(pCamPos == rear_camera_index)
        {
           memcpy(m_rearCamZone,pCamZone,4*sizeof(float));
        }
        else if(pCamPos == left_camera_index)
        {
           memcpy(m_leftCamZone,pCamZone,4*sizeof(float));
        }
        else if(pCamPos == right_camera_index)
        {
           memcpy(m_rightCamZone,pCamZone,4*sizeof(float));
        }
    }

    return GUIDELINE_NORMAL;
}
int HMIGuideLine::SetVisibility(unsigned char pFlag)
{
    if(pFlag > 1)
    {
        pFlag = 1;
    }
    else if(pFlag < 0)
    {
        pFlag = 0;
    }

    m_guideLineVisibility = pFlag;
    m_guideLineNode->SetEnable(pFlag);

    return GUIDELINE_NORMAL;
}
/*===========================================================================*\
 * File Revision History (top to bottom: first revision to last revision)
 *===========================================================================
 *
 *   Date        userid       Description
 * ----------- ----------    -----------
 *  21/01/18   Jensen Wang   Create the HMIGuideLine class.
\*===========================================================================*/
