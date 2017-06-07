
#include "AVMData.h"
#include "SVNode2DStich.h"
#include "SVDelegate.h"
#include "GlSV2D.h"
/*----------------------------------------------


---------------DEFINES-------------------------


----------------------------------------------*/

extern IXrCore* g_pIXrCore;
extern IDeviceManager* rm;

#define GROUND_NODE_WIDTH 200
#define GROUND_NODE_HEIGHT 500
//#define STICH_AERA_WIDTH 216
//#define STICH_AERA_HEIGHT 480
#define STICH_AERA_WIDTH   512	
#define STICH_AERA_HEIGHT 1280


extern float *gf_pgs_para;

extern st_text_box_data_T gpu_debug_texbox;

/****************************************

----------referenced functions-------------------


*************************************************/


/************************************************

---------functions---------------

/************************************************/


extern char c_SV2DVertShaderSrcFileLUT[];   
extern char c_SV2DFragStaticShaderSrcFile[] ;


extern unsigned int Get_Frame_TimeStamp(void);
SVNode2DStich::SVNode2DStich()
{


	
}
void SVNode2DStich::InitStichAreaRT(void)
{

	int meshid[eCarImageMesh+1];
	int modelId,materialID;
	float *pData; 
	unsigned short	 *pIndex;	
	unsigned int BufferSize;
	int j=0;
	ICamera*pCamera;
	float car_rect_image[4];
	float car_rect_adjust[4];
	float car_rect_vertex_array[28] = {
	0,0,0.000000,0.0,0.0,1.0,0.0,
		0,0,0.000000,1.0,0.0,1.0,0.0,
		0,0,0.000000,0.0,1.0,1.0,0.0,
		0,0,0.000000,1.0,1.0,1.0,0.0,


	};
	float *pCarRect;

	char *MeshName[eCarImageMesh+1]={"FrontMesh","RearMesh","LeftMesh","RightMesh","FrontLeftMesh","FrontRightMesh","RearLeftMesh","RearRightMesh","Car"};
	char *Model[eCarImageMesh+1]={"0","1","2","3","4","5","6","7","8"};

	m_StichAreaNodeId = rm->CreateRenderTarget(STICH_AERA_WIDTH, STICH_AERA_HEIGHT, XR_R8G8B8A8, XR_NONE, &m_StichAreaRT);

	char VertexName[MAX_NAME_LENGTH];
	char FragShaderName[MAX_NAME_LENGTH];
	AVMData::GetInstance()->m_cam_source->GetShaderName(VertexName,SV_2D_VERTEX_SHADER);	
	AVMData::GetInstance()->m_cam_source->GetShaderName(FragShaderName,SV_2D_FRAGMENT_SHADER);
	
	m_StichAreaNodeId= g_pIXrCore->CreateRenderNodeScene(m_StichAreaNodeId, &m_StichAreaNode);
	
	


	sv2Ddelegate = new RenderDelegateSV2D();
	
	sv2Ddelegate->Init();
#define Material_MAX 6
	//step 1. prepare for Material ,different camera input image 
	IRenderEffect* pIEffect[Material_MAX];
	int SV2DMTL[Material_MAX];
		float pos[3];
	

	for(int i=0;i<Material_MAX;i++)
	{
		SV2DMTL[i] = m_StichAreaNode->CreateMaterial(Material_Custom, &m_SV2DMtl);
		m_SV2DMtl->CreateMaterialEffect(&pIEffect[i]);
		if(i == 0)
		{
			
			AVMData::GetInstance()->m_cam_source->SetCameraSourceToMaterial(m_SV2DMtl,front_camera_index);	
		}
		else if(i ==1)
		{
			AVMData::GetInstance()->m_cam_source->SetCameraSourceToMaterial(m_SV2DMtl,rear_camera_index);	
		}
		else if(i == 2)
		{
			AVMData::GetInstance()->m_cam_source->SetCameraSourceToMaterial(m_SV2DMtl,left_camera_index);	
		}
		else if(i ==3)
		{
			AVMData::GetInstance()->m_cam_source->SetCameraSourceToMaterial(m_SV2DMtl,right_camera_index);	
		}	
		else if(i ==4)
		{
			
			m_SV2DMtl->SetDiffuseMap(m_groundRT->GetTextureId());	
			
		}	
		else
		{
		
			AVMData::GetInstance()->m_cam_source->SetCameraSourceToMaterial(m_SV2DMtl,right_camera_index);	
		}
		if(i == 4)
		{
			pIEffect[i]->InitShaderFromFile("Effect_SV2DImage", c_SV2DVertShaderSrcFileLUT, c_SV2DFragStaticShaderSrcFile,	sizeof(SV2D_PARAM_CB), XR_VERTEX_LAYOUT_PTAK, 0);
		}
		else
		{
			pIEffect[i]->InitShaderFromFile("Effect_SV2D", VertexName, FragShaderName,	sizeof(SV2D_PARAM_CB), XR_VERTEX_LAYOUT_PTAK, 0);


		}
		pIEffect[i]->SetRenderDelegate(sv2Ddelegate);
	}

	

	  //step 2 prepare mesh for 2D stiching
	for(int i =0;i<8;i++)
	{
		m_SV2DData->GetVertexBuffer(i,&pData,&BufferSize);
		meshid[i] = m_StichAreaNode->CreateMesh(ModelType_Null, 0,0,0,MeshName[i], &m_planeMesh);

		m_planeMesh->LoadVertexFromArray(pData, XR_VERTEX_LAYOUT_PTAK, BufferSize);

		m_planeMesh->SetName(Model[i]);
		m_SV2DData->GetIndexBuffer(i,&pIndex,&BufferSize);		
		m_planeMesh->LoadIndexFromArray(pIndex ,2* BufferSize);
	}

	//step 3 combine mesh and material(video texture) together.
	for(int i =0 ;i<=7;i++)
	{
		if(i<eFrontLeftMesh)
		{
			materialID=i;
		}
		else if(i == eFrontLeftMesh || i == eRearLeftMesh)
		{
		   materialID = 2;
		   
		}
		else if(i<= eRearRightMesh)
		{
			materialID=3;
			
		}
		else if(i == eCarImageMesh)
		{
			materialID =4;
			
		}
		else
		{
			materialID = 5; 	
		}
		
		
		modelId = m_StichAreaNode->CreateModel(0, SV2DMTL[materialID], -1, InsertFlag_Default, 1, 0, 0, 1, &m_SV2Dplane[i+j*eMeshIndexMax]);
		m_SV2Dplane[i+j*eMeshIndexMax]->SetMesh(meshid[i]);
		m_SV2Dplane[i+j*eMeshIndexMax]->SetName(Model[i]);		
		m_SV2Dplane[i+j*eMeshIndexMax]->SetTransitionStyle(500, AnimationStyle_EaseOut, AP_SX | AP_SY);
	}


#if 0
	int i=8;
	 materialID =4;
	modelId = m_StichAreaNode->CreateModel(0, SV2DMTL[materialID], -1, InsertFlag_Default, 1, 0, 0, 1, &m_SV2Dplane[i+j*eMeshIndexMax]);
	m_SV2Dplane[i+j*eMeshIndexMax]->SetMesh(meshid[i]);
	m_SV2Dplane[i+j*eMeshIndexMax]->SetName(Model[i]);		
	m_SV2Dplane[i+j*eMeshIndexMax]->SetTransitionStyle(500, AnimationStyle_EaseOut, AP_SX | AP_SY);
#endif
	/////////////////////////////cameraObject//////////////////
	int cameraId = m_StichAreaNode->CreateCamera(120, Float32(XrGetScreenWidth())/XrGetScreenHeight(), 10.0f, 16000.0f, &pCamera);

	
	pCamera->Set2DCamera(1);
	pCamera->SetPosition(0,0,2000);
	pCamera->LookAt(0.0,0.0,-0.0);
	pCamera->RotateAround(0,45);
	m_StichAreaNode->SetCamera(cameraId);


}
void SVNode2DStich::CalcShadowTextureCoord(float *car_rect,float *shadow_rect,float *pshadow_texture)
{
    float total_width = shadow_rect[rect_right]-shadow_rect[rect_left];
	float total_height = shadow_rect[rect_top] - shadow_rect[rect_bottom];
    pshadow_texture[rect_left]=(car_rect[rect_left]-shadow_rect[rect_left])/total_width;
    pshadow_texture[rect_right]=(car_rect[rect_right]-shadow_rect[rect_left])/total_width;
    pshadow_texture[rect_top]=(car_rect[rect_top]-shadow_rect[rect_bottom])/total_height;
    pshadow_texture[rect_bottom]=(car_rect[rect_bottom]-shadow_rect[rect_bottom])/total_height;

    return;
}
void SVNode2DStich::Init2DGroundNodeRT(int j)
{


	Int32 tempmaterialid;	
    ICamera*pCamera;
    INode *pGroundNode;
	int cameraId;
	XRVertexLayout  data_format;
	Int32 icount;
	int i=0;
    float *pVertexData;

	int ground_id = rm->CreateRenderTarget(GROUND_NODE_WIDTH, GROUND_NODE_HEIGHT, XR_R8G8B8A8, XR_NONE, &m_groundRT);

	
	// create scene Node
	m_GroundNodeId = g_pIXrCore->CreateRenderNodeScene(ground_id, &m_GroundNode);
	
	//m_GroundNode->SetClear(TRUE, TRUE);
    //m_GroundNode->SetClearColor(0.0,0.0,0.0,1.0);



	//create material
	tempmaterialid = m_GroundNode->CreateMaterial(Material_Rigid_Texture, &m_ground_mtl);
	//m_ground_mtl->SetDiffuseMap(m_lisenceRT->GetTextureId());
    // m_ground_mtl->SetDiffuseMap(RADARALARMTEX[j]);
	int MeshId = m_GroundNode->CreateMesh(ModelType_Plane_Dynamic,2,  2,0, "ground", &m_pMeshStichGround);
	int groundId = m_GroundNode->CreateModel(0, tempmaterialid, -1, InsertFlag_Default,0,0,0, 1.0, &pGroundNode);

    m_pMeshStichGround->LockData(&pVertexData,&data_format,&icount);
	pVertexData[0]=-1.0;
	pVertexData[1]=-1.0;
	pVertexData[2]=0.0;
	pVertexData[8]=1.0;
	pVertexData[9]=-1.0;
    pVertexData[10]=0.0;	
	pVertexData[16]=-1.0;
	pVertexData[17]=1.0;
	pVertexData[18]=0.0;
	pVertexData[24]=1.0;
	pVertexData[25]=1.0;
	pVertexData[26]=0.0;
 
    m_pMeshStichGround->UnLockData();

	cameraId = m_GroundNode->CreateCamera(120, Float32(XrGetScreenWidth())/XrGetScreenHeight(), 10.0f, 16000.0f, &pCamera);
	pCamera->Set2DCamera(1);
	pCamera->SetPosition(0,0,2000);
	pCamera->LookAt(0.0,1000.0,-0.0);
	pCamera->RotateAround(0,45);
	m_GroundNode->SetCamera(cameraId);

    pGroundNode->SetMesh(MeshId);

}
void SVNode2DStich::InitStichKeyFrameNodeRT(int j)
{

	int meshid[eCarImageMesh+1];
	int modelId,materialID;
	float *pData; 
	unsigned short	 *pIndex;	
	unsigned int BufferSize;
	IMaterial *pTempMtl;
	IMesh *pMeshTemp;
	INode *pStichNode;
	ICamera*pCamera;
	XRVertexLayout  data_format;
	Int32 icount;
	int i=0;
    float *pVertexData;
	float car_rect_image[4];
	float car_rect_adjust[4];
	float car_rect_vertex_array[28] = {
	0,0,0.000000,0.0,0.0,1.0,1.0,
		0,0,0.000000,1.0,0.0,1.0,1.0,
		0,0,0.000000,0.0,1.0,1.0,1.0,
		0,0,0.000000,1.0,1.0,1.0,1.0,


	};
	
	float texture[4]={0.0,1.0,1.0,0.0};
	float *pCarRect;

	char *MeshName[eCarImageMesh+1]={"FrontMesh","RearMesh","LeftMesh","RightMesh","FrontLeftMesh","FrontRightMesh","RearLeftMesh","RearRightMesh","Car"};
	char *Model[eCarImageMesh+1]={"0","1","2","3","4","5","6","7","a"};

	m_StichNodeId[j] = rm->CreateRenderTarget(STICH_AERA_WIDTH, STICH_AERA_HEIGHT, XR_R8G8B8A8, XR_NONE, &m_StichNodeRT[j]);

	char VertexName[MAX_NAME_LENGTH];
	char FragShaderName[MAX_NAME_LENGTH];
	AVMData::GetInstance()->m_cam_source->GetShaderName(VertexName,SV_2D_VERTEX_SHADER);	
	AVMData::GetInstance()->m_cam_source->GetShaderName(FragShaderName,SV_2D_FRAGMENT_SHADER);
	
	m_StichNodeId[j]= g_pIXrCore->CreateRenderNodeScene(m_StichNodeId[j], &m_StichNode[j]);
	

//all stich area
	//create material
	int tempmaterialid = m_StichNode[j]->CreateMaterial(Material_Rigid_Texture, &pTempMtl);
	pTempMtl->SetDiffuseMap(m_StichAreaRT->GetTextureId());
	int MeshId =  m_StichNode[j]->CreateMesh(ModelType_Plane_Dynamic,2,  2,0, "ground", &pMeshTemp);
	int groundId = m_StichNode[j]->CreateModel(0, tempmaterialid, -1, InsertFlag_Default,0,0,0, 1.0, &pStichNode);
	

    pMeshTemp->LockData(&pVertexData,&data_format,&icount);
	pVertexData[0]=-1.0;
	pVertexData[1]=-1.0;
	pVertexData[2]=0.0;
	pVertexData[6]=texture[rect_left];
	pVertexData[7]=texture[rect_bottom];
	pVertexData[8]=1.0;
	pVertexData[9]=-1.0;
    pVertexData[10]=0.0;	
	pVertexData[14]=texture[rect_right];
	pVertexData[15]=texture[rect_bottom];
	pVertexData[16]=-1.0;
	pVertexData[17]=1.0;			
	pVertexData[18]=0.0;
	
	pVertexData[22]=texture[rect_left];
	pVertexData[23]=texture[rect_top];
	pVertexData[24]=1.0;
	pVertexData[25]=1.0;
	pVertexData[26]=0.0;
 	
	pVertexData[30]=texture[rect_right];
	pVertexData[31]=texture[rect_top];
    pMeshTemp->UnLockData();
	pStichNode->SetMesh(MeshId);
	sv2Ddelegate = new RenderDelegateSV2D();
	
	sv2Ddelegate->Init();

	//step 2. create material
	IRenderEffect* pIEffect;
	int SV2DMTL;
	float pos[3];
	

	SV2DMTL = m_StichNode[j]->CreateMaterial(Material_Custom, &m_SV2DMtl);
	m_SV2DMtl->CreateMaterialEffect(&pIEffect);

	m_SV2DMtl->SetDiffuseMap(m_groundRT->GetTextureId());	
	pIEffect->InitShaderFromFile("Effect_SV2DImage_TEST", c_SV2DVertShaderSrcFileLUT, c_SV2DFragStaticShaderSrcFile,	sizeof(SV2D_PARAM_CB), XR_VERTEX_LAYOUT_PTAK, 0);	
	pIEffect->SetRenderDelegate(sv2Ddelegate);

	
	//adjust shadow position
	m_SV2DData->GetVertexBuffer(8,&pCarRect,&BufferSize);
	memcpy(car_rect_vertex_array,pCarRect,BufferSize*7*sizeof(float));
	for (int i =0; i<4;i++)
	{
		AVMData::GetInstance()->m_2D_lut->GetCarRect(&car_rect_image[i],i);
		AVMData::GetInstance()->m_2D_lut->GetCarShadowAdjust(&car_rect_adjust[i],i);
		if(j != 2)
		{
		    car_rect_image[i]=car_rect_image[i]+car_rect_adjust[i];
		}
		else
		{
		    car_rect_adjust[i]=car_rect_image[i]+car_rect_adjust[i];
		
		}
	}
	 int test[2];
    if(j!=2)
    {
    	Cvt4Point2Rect(car_rect_image,m_CarPoint_Image);
    	for(int i=0;i<4;i++)
    	{
			test[0] = ((m_CarPoint_Image[i].x+1)+1.0/STICH_AERA_WIDTH)/(2.0/STICH_AERA_WIDTH);
			test[1] = ((m_CarPoint_Image[i].y+1)+1.0/STICH_AERA_HEIGHT)/(2.0/STICH_AERA_HEIGHT);
			m_CarPoint_Image[i].x=(float)test[0]*(2.0/STICH_AERA_WIDTH)-1;
			m_CarPoint_Image[i].y=(float)test[1]*(2.0/STICH_AERA_HEIGHT)-1;
    		car_rect_vertex_array[i*7+0] = m_CarPoint_Image[i].x;
    		car_rect_vertex_array[i*7+1] = m_CarPoint_Image[i].y;
			
    	}
    }
	else
	{
	
	    CvPoint2D32f CarPoint_Image_Temp[4];
		
	    CvPoint2D32f CarPoint_Image_Texture[4];
    	CalcShadowTextureCoord(car_rect_image,car_rect_adjust,texture);
	    Cvt4Point2Rect(car_rect_image,CarPoint_Image_Temp);
		
	    Cvt4Point2Rect(texture,CarPoint_Image_Texture);
		for(int i=0;i<4;i++)
    	{
    		car_rect_vertex_array[i*7+0] = CarPoint_Image_Temp[i].x;
    		car_rect_vertex_array[i*7+1] = CarPoint_Image_Temp[i].y;
    		car_rect_vertex_array[i*7+3] = CarPoint_Image_Texture[i].x;
    		car_rect_vertex_array[i*7+4] = 1.0-CarPoint_Image_Texture[i].y;

			
    	}
	}
	meshid[8] = m_StichNode[j]->CreateMesh(ModelType_Null, 0,0,0,MeshName[8], &m_planeMesh);
	m_planeMesh->LoadVertexFromArray(car_rect_vertex_array, XR_VERTEX_LAYOUT_PTAK, BufferSize);
	m_planeMesh->SetName(Model[8]);
	m_SV2DData->GetIndexBuffer(8,&pIndex,&BufferSize);		
	m_planeMesh->LoadIndexFromArray(pIndex ,2* BufferSize);


	//step 3 combine mesh and material(video texture) together.

	i=8;
	 materialID =4;
	modelId = m_StichNode[j]->CreateModel(0, SV2DMTL, -1, InsertFlag_Default, 1, 0, 0, 1, &m_SV2Dplane[i+j*eMeshIndexMax]);
	m_SV2Dplane[i+j*eMeshIndexMax]->SetMesh(meshid[i]);
	m_SV2Dplane[i+j*eMeshIndexMax]->SetName(Model[i]);		
	m_SV2Dplane[i+j*eMeshIndexMax]->SetTransitionStyle(500, AnimationStyle_EaseOut, AP_SX | AP_SY);

	/////////////////////////////cameraObject//////////////////
	int cameraId = m_StichNode[j]->CreateCamera(120, Float32(XrGetScreenWidth())/XrGetScreenHeight(), 10.0f, 16000.0f, &pCamera);

	
	pCamera->Set2DCamera(1);
	pCamera->SetPosition(0,0,2000);
	pCamera->LookAt(0.0,0.0,-0.0);
	pCamera->RotateAround(0,45);
	m_StichNode[j]->SetCamera(cameraId);


}

int SVNode2DStich::ProcessGroundCoord(float steering_wheel_angle,float vehicle_speed,float left_wheel_speed, float right_wheel_speed,unsigned char gear_state,int time_offset,float yaw_rate)
{
    COMMON_VEHICLE_DATA_SIMPLE vehicle_state;
	float *pTransformMatrix;
	float car_rect_image[4];
	float car_rect_world[4];
	float car_rect_adjust[4];
	CvPoint2D32f CarPoint[4];
	static int pre_time=0;
	static int init_flag = 0;
	int present_time=0;
	CvPoint2D32f WorldInPoint[4];
	CvPoint2D32f WorldOutPoint[4];
	
	int update_key_frame_flag;

    present_time = XrGetTime();
		
	vehicle_state.steering_angle =steering_wheel_angle;
	vehicle_state.vehicle_speed = vehicle_speed;
	vehicle_state.wheel_speed_fl = left_wheel_speed;
	vehicle_state.wheel_speed_rl = left_wheel_speed;
	vehicle_state.wheel_speed_fr = right_wheel_speed;
	vehicle_state.wheel_speed_rr = right_wheel_speed;
	vehicle_state.shift_pos = gear_state;
	vehicle_state.yaw_rate = yaw_rate;
	//m_vehicle_motion->revMotion2KframePredictVCS(vehicle_state,40000,m_track,m_t,m_Move_Matrix,update_key_frame_flag,0.3);
	if(init_flag ==0)
	{
	    m_vehicle_motion->revMotion2KframePredictVCS(vehicle_state,40000,m_track,m_t,m_Move_Matrix,update_key_frame_flag,0.02);
	}
    else
    {
		if(vehicle_state.yaw_rate >2.5 || vehicle_state.yaw_rate<-2.5)
        {
           //printf("yaw_rate = %f",yaw_rate);
	       m_vehicle_motion->revMotion2KframePredictVCS(vehicle_state,(int32_t)Get_Frame_TimeStamp(),m_track,m_t,m_Move_Matrix,update_key_frame_flag,0.3);
        }
		else
		{
		
	//	printf("yaw_rate = %f",yaw_rate);
			m_vehicle_motion->revMotion2KframePredictVCS(vehicle_state,(int32_t)Get_Frame_TimeStamp(),m_track,m_t,m_Move_Matrix,update_key_frame_flag,0.02);

		}	   
	  // m_vehicle_motion->Motion2KframePredictVCS(vehicle_state,40000,m_track,m_t,m_Move_Matrix,update_key_frame_flag,0.3);
         
    }
	sprintf(gpu_debug_texbox.text_content[2],"frame_time %d",Get_Frame_TimeStamp());

	init_flag = 1;
	pre_time = present_time;
	#if 0
    for (int i =0; i<4;i++)
    {
	    AVMData::GetInstance()->m_2D_lut->GetCarRect(&car_rect_image[i],i);
		AVMData::GetInstance()->m_2D_lut->GetCarShadowAdjust(&car_rect_adjust[i],i);
		car_rect_image[i]=car_rect_image[i]+car_rect_adjust[i];
    }
	 

	Cvt4Point2Rect(car_rect_image,CarPoint);
	#endif
    for (int i =0;i<4;i++)
    {
        AVMData::GetInstance()->m_2D_lut->CvtPointImage2Wolrd(m_CarPoint_Image[i],&WorldInPoint[i]);

		m_vehicle_motion->getPos2Kframe(WorldInPoint[i],m_Move_Matrix,WorldOutPoint[i]);
    
	    AVMData::GetInstance()->m_2D_lut->CvtPointWorld2Image(WorldOutPoint[i],&m_Car_rect[i]);

    }
    
    return update_key_frame_flag;
		
}

void SVNode2DStich::Cvt4Point2Rect(float *pPoint,CvPoint2D32f *pRect)
{
     pRect[LEFT_BOTTOM_POINT_INDEX].x=pPoint[rect_left];
	 pRect[LEFT_BOTTOM_POINT_INDEX].y=pPoint[rect_bottom];

     pRect[RIGHT_BOTTOM_POINT_INDEX].x=pPoint[rect_right];
     pRect[RIGHT_BOTTOM_POINT_INDEX].y=pPoint[rect_bottom];

     pRect[LEFT_TOP_POINT_INDEX].x=pPoint[rect_left];
	 pRect[LEFT_TOP_POINT_INDEX].y=pPoint[rect_top];

     pRect[RIGHT_TOP_POINT_INDEX].x=pPoint[rect_right];
     pRect[RIGHT_TOP_POINT_INDEX].y=pPoint[rect_top];	 

}
void SVNode2DStich::Update2DStichRslt(void)
{
    IMesh *pMeshTemp;

    for(int i=0;i<8;i++)
    {
    
    	m_SV2Dplane[i]->GetMesh(&pMeshTemp);
    	pMeshTemp->UnLockData();
    
    }
 

}

#if 0
void SVNode2DStich::CvtPointImage2Wolrd(CvPoint2D32f InPoint,CvPoint2D32f *pOutPoint)
{
     pOutPoint->x =  -((-InPoint.y*240+240)-m_calib[POS_CALIB_CX])*m_calib[POS_CALIB_PPMMX]/1000.0;
	 pOutPoint->y = ((InPoint.x*108+108)-m_calib[POS_CALIB_CY])*m_calib[POS_CALIB_PPMMY]/1000.0;

}
void SVNode2DStich::CvtPointWorld2Image(CvPoint2D32f InPoint,CvPoint2D32f *pOutPoint)
{

	 pOutPoint->y = 1.0-(((InPoint.x*1000.0)/(0.0-m_calib[POS_CALIB_PPMMX])+m_calib[POS_CALIB_CX])/240.0);
	  pOutPoint->x =((InPoint.y*1000.0)/m_calib[POS_CALIB_PPMMY]+m_calib[POS_CALIB_CY]-108.0)/108.0;

}
#endif


void SVNode2DStich::UpdateGoundTextureCoord(CvPoint2D32f *pTextureIndex)
{
    float *pVertexData;
	XRVertexLayout  data_format;
	Int32 icount;
	int i=0;
	IMaterial* pTempMtl;
	static int cnt=0;
	



	
    m_pMeshStichGround->LockData(&pVertexData,&data_format,&icount);

	int test[2];

    for(i=0;i<icount;i++)
    {
  		test[0] = ((pTextureIndex[i].x+1)/2.0+0.5/STICH_AERA_WIDTH)/(1.0/STICH_AERA_WIDTH);
		test[1] = ((pTextureIndex[i].y+1)/2.0+0.5/STICH_AERA_HEIGHT)/(1.0/STICH_AERA_HEIGHT);
		pVertexData[8*i+6]=(float)test[0]*(1.0/STICH_AERA_WIDTH);
        pVertexData[8*i+7]=(float)test[1]*(1.0/STICH_AERA_HEIGHT);
        //pVertexData[8*i+6]=(pTextureIndex[i].x+1)/2.0;
        //pVertexData[8*i+7]=(pTextureIndex[i].y+1)/2.0;		

    }
 
		
		m_pMeshStichGround->UnLockData();
}


int SVNode2DStich::Init()
{
	m_SV2DData = new GlSV2D;
    m_SV2DData->Init();
     Init2DGroundNodeRT(0);
	 InitStichAreaRT();
	 InitStichKeyFrameNodeRT(0);
	 InitStichKeyFrameNodeRT(1);
	 
	 InitStichKeyFrameNodeRT(2);
     for(int i =0;i<4;i++)
     {
         m_calib[i] = gf_pgs_para[i];
     }
	 for(int i =0;i<9;i++)
	 {
	     m_transform_step[i]=0;
		 if(i == 0||i==4||i==8)
		 {
		     m_t[i]=1.00;
		     m_Move_Matrix[i]=1.0;
		 }
		 else
		 {

		     m_t[i]=0;
		     m_Move_Matrix[i]=0;
		 }
	 }
	 m_vehicle_motion = new VehicleMotion;

	 m_transform_step[0]=1;
	 m_transform_step[3]=1;
	 m_transform_step[8]=1;
	 m_transform_matrix=XRMat3::Scale(1.0,1.0,1.0);
	 m_key_frame_index = 0;
	 m_init_flag = 0;
	 	 m_track=0;


	 return 0;
}


int SVNode2DStich::Update(float steering_wheel_angle,float vehicle_speed,float left_wheel_speed,float right_wheel_speed,unsigned char gear_state,int time_offset,float yaw_rate)
{
    static unsigned int cnt = 0;
    float pTemp[8]={0.2,0.2,0.8,0.2,0.2,1.0,0.8,1.0};
	unsigned char update_key_flag=0;
	static unsigned char update_key_frame_sures = 0;
	static unsigned char last_text_state = 0;
	
   static unsigned int update_time = 0;
	
	update_key_flag = ProcessGroundCoord(steering_wheel_angle,vehicle_speed,left_wheel_speed,right_wheel_speed,gear_state,time_offset,yaw_rate);
	UpdateGoundTextureCoord(m_Car_rect);	

	
	sprintf(gpu_debug_texbox.text_content[0],"left_wheel%f,",left_wheel_speed);
	
	sprintf(gpu_debug_texbox.text_content[1],"right_wheel %f,",right_wheel_speed);
	//sprintf(gpu_debug_texbox.text_content[2],"time_offset %d",time_offset);
	
	sprintf(gpu_debug_texbox.text_content[3],"update_frame_time %d",update_time);
	if(update_key_frame_sures != update_key_flag&&update_key_flag == 1)
	{
	    if(last_text_state == 0)
	    {
	    
		    last_text_state = 1;
	        sprintf(gpu_debug_texbox.text_content[4],"Key_frame_updated !");
	    }
		else
		{
		 
	        sprintf(gpu_debug_texbox.text_content[4],"Key_frame_updated  again!");
		    last_text_state = 0;
		}
	}
	
	sprintf(gpu_debug_texbox.text_content[5],"point (%f,%f)",m_Car_rect[0].x,m_Car_rect[0].y);
	update_time++;

    if(m_init_flag == 0)
    {
        m_key_frame_index = 0;
		m_ground_mtl->SetDiffuseMap(m_StichNodeRT[m_key_frame_index]->GetTextureId());
		m_SV2Dplane[eCarImageMesh+m_key_frame_index*eMeshIndexMax]->SetEnable(1);
		m_StichNode[m_key_frame_index]->SetEnable(1);
		m_StichNode[1-m_key_frame_index]->SetEnable(1);
		
	

        m_init_flag = 1;
    }
	else
	{
	    if(m_init_flag<10)
	    {
	        update_key_flag =1;
			m_init_flag++;
	    
	    }
	
        if(update_key_flag == 1)
        {
            m_key_frame_index = 1- m_key_frame_index;
                         
   		  m_ground_mtl->SetDiffuseMap(m_StichNodeRT[m_key_frame_index]->GetTextureId());
		   m_SV2Dplane[eCarImageMesh+(m_key_frame_index)*eMeshIndexMax]->SetEnable(1);
    	  m_SV2Dplane[eCarImageMesh+(1-m_key_frame_index)*eMeshIndexMax]->SetEnable(1);
    	  m_StichNode[m_key_frame_index]->SetEnable(0);
    	  m_StichNode[1-m_key_frame_index]->SetEnable(1);

		}
		else
		{
		
		   //m_StichNode[m_key_frame_index]->SetEnable(0);
		}
		
		m_StichNode[m_key_frame_index]->SetEnable(0);
		//m_StichNode[m_key_frame_index]->SetEnable(1);

	
	}

	
	update_key_frame_sures = update_key_flag;


	return 0;

}
int SVNode2DStich::GetKeyFrameTextureId()
{
    return m_StichNodeRT[m_key_frame_index]->GetTextureId();
}
int SVNode2DStich::GetGroundTextureId()
{
    return m_groundRT->GetTextureId();
}
int SVNode2DStich::GetStichFrameTextureId()
{
    return m_StichNodeRT[2]->GetTextureId();
   // return m_groundRT->GetTextureId();
}
int SVNode2DStich::SetEnable(unsigned char flag)
{
    if(flag ==0)
    {
	    m_StichNode[0]->SetEnable(0);
		
	    m_StichNode[1]->SetEnable(0);
		
	    m_GroundNode->SetEnable(0);
    }
	else
    {
	    m_StichNode[0]->SetEnable(1);
		
	    m_StichNode[1]->SetEnable(1);
		
	    m_GroundNode->SetEnable(1);
    }		
   // return m_groundRT->GetTextureId();
	return 0;
}

