#include <fbxsdk.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <sstream>
#include <ctime>

#include "print.h"
#include "common.h"

/////////////////////////////////////////////////////////////////////////
//
// This program converts any file in a format supported by the FBX SDK 
// into DAE, FBX, 3DS, OBJ and DXF files. 
//
// Steps:
// 1. Initialize SDK objects.
// 2. Load a file(fbx, obj,...) to a FBX scene.
// 3. Create a exporter.
// 4. Retrieve the writer ID according to the description of file format.
// 5. Initialize exporter with specified file format
// 6. Export.
// 7. Destroy the exporter
// 8. Destroy the FBX SDK manager
//
/////////////////////////////////////////////////////////////////////////

using namespace std;

/* Tab character ("\t") counter */
int numTabs = 0;

const char* lFileTypes[] =
{
    "_dae.dae",            "Collada DAE (*.dae)",
    "_fbx7binary.fbx", "FBX binary (*.fbx)",
    "_fbx7ascii.fbx",  "FBX ascii (*.fbx)",
    "_fbx6binary.fbx", "FBX 6.0 binary (*.fbx)",
    "_fbx6ascii.fbx",  "FBX 6.0 ascii (*.fbx)",
    "_obj.obj",            "Alias OBJ (*.obj)",
    "_dxf.dxf",            "AutoCAD DXF (*.dxf)"
};
vector<FbxVector4> MyObjLoader(string& filePath) {
    ifstream File(filePath, ios::in);
    if (!File.is_open()) {
        cout << "File cannot be imported!" << endl;
    }
    int idx = 0;
    int vertsNum = 3896;
    std::string line;
    std::vector< FbxVector4 >verts(vertsNum);
    while (getline(File, line)) {
        if (line.substr(0, 2) == "v ") {
            std::istringstream v(line.substr(2));
            double x, y, z;
            v >> x >> y >> z;
            verts[idx] = FbxVector4(x, y, z);
            idx++;
        }
    }
    File.close();
    return verts;
}
vector<int>TXTLoader(string& filePath) {
    ifstream File(filePath, ios::in);
    if (!File.is_open()) {
        cout << "File cannot be imported!" << endl;
    }
    int idx = 0;
    std::string line;
    std::vector<int>verts;
    while (getline(File, line)) {
        verts.push_back(atoi(line.c_str()));
    }
    File.close();
    return verts;
}
int main(int argc, char** argv)
{
    
    timespec t0, t1;
    clock_gettime(CLOCK_REALTIME, &t0);

    std::string rootPath = "/home/pcd/vscodes/fbx_synthesizer";
    // std::string originalFbxFile = rootPath + "/data/joy.fbx";
    std::string originalFbxFile = rootPath + "/data/preprocessed.fbx";

    std::string defaultInputPath = "/home/pcd/vscodes/pcd_nricp/result";
    std::string reconFolder = defaultInputPath + "/blendshapes";
    std::string newBaseFile = defaultInputPath + "/result.obj";

    std::string outputFbxFile = rootPath + "/result/output.fbx";
    
    std::string EyeIndicesFile = rootPath + "/data/eyes.txt";
    std::string MouthIndicesFile = rootPath + "/data/mouth.txt";
    
    vector<int> EyeIndices = TXTLoader(EyeIndicesFile);
    vector<int> MouthIndices = TXTLoader(MouthIndicesFile);

    // accept 1 par as outputFile
    if(argc == 2){
        outputFbxFile = string(argv[1]);
        cout << "specify outputFbxFile as " << outputFbxFile << endl;
    }

    std::vector<std::string> reconFiles;
    std::vector<std::string> bsName;
    for (auto& file : std::filesystem::directory_iterator(reconFolder)){
        reconFiles.push_back(file.path());
    }
    for (auto& file : reconFiles) {
        int slash_pos = file.rfind("/");
        int dot_pos = file.find(".");
        bsName.push_back(file.substr(slash_pos+1,dot_pos-slash_pos-1));
    }
    //for (auto& file : reconFiles) {
    //    std::cout << file << std::endl;
    //}
    //for (auto& bs : bsName) {
    //    std::cout << bs << std::endl;
    //}
    

    int lFormat = 1;
    
    FbxManager* lSdkManager = NULL;
    FbxScene* lCompositeScene = NULL;
    
    // initialize CompositeScene
    InitializeSdkObjects(lSdkManager, lCompositeScene);

    // Set Output file format
    lFormat = lSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription("FBX ascii (*.fbx)");

    // ��ԭʼfbxfileװ��composite scene
    bool lResult = LoadScene(lSdkManager, lCompositeScene, originalFbxFile.c_str());

    // print loaded scene nodes
    numTabs = 0;
    GetNodeRecursive(lCompositeScene->GetRootNode(), &numTabs);

    FbxNode* Target = lCompositeScene->GetRootNode()->FindChild("Face");
    FbxMesh* Mesh = Target->GetMesh();
    
    int vertsNum = 3896;
    vector<FbxVector4> newBase = MyObjLoader(newBaseFile);
    for (int i = 0; i < vertsNum; i++)
        Mesh->GetControlPoints()[i] = newBase[i];

    FbxVector4 EyesLocal = FbxVector4(-4.40223, 216.102, 6.55363);
    FbxVector4 MouthLocal = FbxVector4(0.022379, 109.13, 4.15885);

    // new base verts
    FbxVector4 EyesNew = FbxVector4(0, 0, 0);
    for (int i = 0; i < EyeIndices.size(); i++) {
        EyesNew += newBase[EyeIndices[i]];
        // FbxVector4 tmp = newBase[EyeIndices[i]];
        // printf("Offsets are :%lf %lf %lf\n", tmp[0], tmp[1], tmp[2]);
    }
    FbxVector4 MouthNew = FbxVector4(0, 0, 0);
    for (int i = 0; i < MouthIndices.size(); i++) {
        MouthNew += newBase[MouthIndices[i]];
    }
    
    FbxVector4 EyesOffset = (EyesNew - EyesLocal) / EyeIndices.size();
    FbxVector4 MouthOffset = (MouthNew - MouthLocal) / MouthIndices.size();
    EyesOffset *= 100;
    MouthOffset *= 100;
    // FbxNode* EyeBallLeft = lCompositeScene->GetRootNode()->FindChild("EyeballLeft");
    // FbxDouble3 EyeBallLeftTranslation = EyeBallLeft->LclTranslation.Get();
    // printf("Original Translations are: %lf %lf %lf \n",EyeBallLeftTranslation[0],EyeBallLeftTranslation[1],EyeBallLeftTranslation[2]);
    // // 左眼与右眼 x轴位移相反
    // EyeBallLeft->LclTranslation.Set(FbxDouble3(-EyesOffset[0], EyesOffset[2], EyesOffset[1]));
    // FbxNode* EyeBallRight = lCompositeScene->GetRootNode()->FindChild("EyeballRight");
    // // maya坐标系与blender、obj坐标系转换: y轴与z轴互换！！
    // EyeBallRight->LclTranslation.Set(FbxDouble3(EyesOffset[0], EyesOffset[2], EyesOffset[1]));
    
    FbxNode* TeethLower = lCompositeScene->GetRootNode()->FindChild("TeethLower");
    FbxDouble3 CommonTrans = TeethLower->LclTranslation.Get();
    CommonTrans = FbxDouble3(CommonTrans[0] + MouthOffset[0], CommonTrans[1] + MouthOffset[2], CommonTrans[2] + MouthOffset[1]);
    TeethLower->LclTranslation.Set(CommonTrans);
    FbxNode* TeethUpper = lCompositeScene->GetRootNode()->FindChild("TeethUpper");
    TeethUpper->LclTranslation.Set(CommonTrans);
    FbxNode* Tongue = lCompositeScene->GetRootNode()->FindChild("Tongue");
    Tongue->LclTranslation.Set(CommonTrans);
    FbxNode* Cavity = lCompositeScene->GetRootNode()->FindChild("Cavity");
    Cavity->LclTranslation.Set(FbxDouble3(MouthOffset[0], MouthOffset[2], MouthOffset[1]));
    
    // ��ȡblendshape����ʽ
    //FbxBlendShape* BlendShape = (FbxBlendShape*)Target->GetGeometry()->GetDeformer(0, FbxDeformer::eBlendShape);
    
    // ɾ��ԭ��blendshape deformer
    if(Mesh->GetDeformerCount() == 1)
        Target->GetMesh()->RemoveDeformer(0);

    // �����µ�blendshape deformer
    FbxBlendShape* BlendShape = FbxBlendShape::Create(lCompositeScene, "synthesisBlendShape");
    Target->GetMesh()->AddDeformer(BlendShape);

    for (int i = 0; i < bsName.size(); i++) {       
        FbxShape* newShape = FbxShape::Create(lCompositeScene, bsName[i].c_str());

        newShape->InitControlPoints(vertsNum);

        vector<FbxVector4> points = MyObjLoader(reconFiles[i]);

        for (int i = 0; i < vertsNum; i++) {
            newShape->GetControlPoints()[i] = points[i];
        }
        // add new channel to FbxBlendShape
        FbxBlendShapeChannel* NewChannel = FbxBlendShapeChannel::Create(lCompositeScene, bsName[i].c_str());
        BlendShape->AddBlendShapeChannel(NewChannel);

        NewChannel->AddTargetShape(newShape);
        NewChannel->DeformPercent.Set(0);
    }

    // final deformer count
    int deformerCount = Target->GetMesh()->GetDeformerCount();
    std::cout << "Final mesh has " << deformerCount << " deformers" << endl;

    if(lResult)
    {
        // Create an exporter.
        FbxExporter* lExporter = FbxExporter::Create(lSdkManager, "");

        // Initialize the exporter.
        lResult = lExporter->Initialize(outputFbxFile.c_str(), lFormat, lSdkManager->GetIOSettings());
        if (!lResult)
        {
            FBXSDK_printf("%s:\tCall to FbxExporter::Initialize() failed.\n", lFileTypes[lFormat]);
            FBXSDK_printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
        }
        else
        {
            // Export the scene.
            lResult = lExporter->Export(lCompositeScene);
            if (!lResult)
            {
                FBXSDK_printf("Call to FbxExporter::Export() failed.\n");
            }
        }

        // Destroy the exporter.
        lExporter->Destroy();
    }
    else
    {
        FBXSDK_printf("Call to LoadScene() failed.\n");
    }

    // Delete the FBX SDK manager. All the objects that have been allocated 
    // using the FBX SDK manager and that haven't been explicitly destroyed 
    // are automatically destroyed at the same time.
    DestroySdkObjects(lSdkManager, lResult);


    clock_gettime(CLOCK_REALTIME, &t1);
    double time_ms = t1.tv_sec * 1000 + t1.tv_nsec / 1000000.0 - (t0.tv_sec * 1000 + t0.tv_nsec / 1000000.0);
    std::cout << "Total Used Time: " << time_ms / 1000 << "s" << endl;
    
    
    return 0;
}

