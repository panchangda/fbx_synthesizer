#include <fbxsdk.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <sstream>

#include "print.h"
#include "common.h"

using namespace std;

int numTabs = 0;
// eyeBlinkLeft4.obj -> eyeBlinkLeft.obj
void RemoveNumbersBeforeDot(std::string& folderPath){
    vector<string> filePaths;
    for (auto& file : std::filesystem::directory_iterator(folderPath)){
        filePaths.push_back(file.path());
    }
    for (auto& file : filePaths) {
        string newFileName = file;
        newFileName.erase(file.size()-5,1);
        cout << file << endl << newFileName << endl;
        int flag = rename(file.c_str(), newFileName.c_str());
        if(flag)
            cout<<"fail!"<<endl;
    }
}

vector<FbxVector4> MyObjLoader(string& filePath) {
    ifstream File(filePath, ios::in);
    if (!File.is_open()) {
        cout << "File cannot be imported!" << endl;
    }
    std::string line;
    std::vector< FbxVector4 >verts;
    while (getline(File, line)) {
        if (line.substr(0, 2) == "v ") {
            std::istringstream v(line.substr(2));
            double x, y, z;
            v >> x >> y >> z;
            verts.push_back(FbxVector4(x, y, z));
        }
    }
    File.close();
    return verts;
}
vector<string> GetFileInFolder(string& filePath){
    std::vector<std::string> reconFiles;
    for (auto& file : std::filesystem::directory_iterator(filePath)){
        reconFiles.push_back(file.path());
    }
    return reconFiles;
}
vector<string> GetFileName(vector<string>& files){
    vector<string>bsName;
    for (auto& file : files) {
        int slash_pos = file.rfind("/");
        int dot_pos = file.rfind(".");
        bsName.push_back(file.substr(slash_pos+1,dot_pos-slash_pos-1));
    }
    return bsName;
}
int main(){

    // string folderPath ="../data/blendshapes/EyeballLeft";

    // RemoveNumbersBeforeDot(folderPath);

    // folderPath ="../data/blendshapes/EyeballRight";
    // RemoveNumbersBeforeDot(folderPath);

    string originalFbxFile = "../data/joy.fbx";
    string outputFbxFile = "../data/preprocessed.fbx";

    int lFormat = 1;
    
    FbxManager* lSdkManager = NULL;
    FbxScene* lCompositeScene = NULL;
    
    // initialize CompositeScene
    InitializeSdkObjects(lSdkManager, lCompositeScene);

    // Set Output file format
    lFormat = lSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription("FBX binary (*.fbx)");

    bool lResult = LoadScene(lSdkManager, lCompositeScene, originalFbxFile.c_str());

    // print loaded scene nodes
    numTabs = 0;
    GetNodeRecursive(lCompositeScene->GetRootNode(), &numTabs);

    vector<string>FaceComponents = {"EyeballLeft","EyeballRight","TeethUpper","TeethLower","Tongue"};
    for(auto FaceComponent : FaceComponents){        
        FbxNode* Target = lCompositeScene->GetRootNode()->FindChild(FaceComponent.c_str());
        FbxMesh* Mesh = Target->GetMesh();
        string newBaseFile = "../data/blendshapes/" + FaceComponent + ".obj";
        
        int vertsNum = Mesh->GetControlPointsCount();
        vector<FbxVector4> newBase = MyObjLoader(newBaseFile);
        // for (int i = 0; i < vertsNum; i++){

        //     if(i == 0){
        //         cout<<FaceComponent<<":"<<endl;
        //         FbxVector4 ori = Mesh->GetControlPoints()[i];
        //         printf("%f %f %f", ori[0], ori[1], ori[2]);
        //     }


        //     if(FaceComponent == "EyeballLeft" || FaceComponent == "EyeballRight")            
        //         Mesh->GetControlPoints()[i] = FbxVector4(newBase[i][0], newBase[i][2], -newBase[i][1]);
        //     else
        //         Mesh->GetControlPoints()[i] = FbxVector4((newBase[i][0] + 0.070949)/100, (newBase[i][2] + 7.810629)/100, -(newBase[i][1]+0.152204)/100);
            
        //     if(i == 0){
        //         cout<<" -> ";
        //         FbxVector4 ori = Mesh->GetControlPoints()[i];
        //         printf("%f %f %f\n", ori[0], ori[1], ori[2]);
        //     }
        // }

        if(Mesh->GetDeformerCount() == 1)
            Target->GetMesh()->RemoveDeformer(0);
        FbxBlendShape* BlendShape = FbxBlendShape::Create(lCompositeScene, (FaceComponent + "Blendshape").c_str());
        Target->GetMesh()->AddDeformer(BlendShape);

        string newBSFolder = "../data/blendshapes/" + FaceComponent;
    
        vector<string> reconFiles = GetFileInFolder(newBSFolder);
        cout<<reconFiles[1]<<endl;
        vector<string> bsName = GetFileName(reconFiles);
        cout<<bsName[1]<<endl;
        for (int i = 0; i < bsName.size(); i++) {       
            FbxShape* newShape = FbxShape::Create(lCompositeScene, bsName[i].c_str());

            newShape->InitControlPoints(vertsNum);

            vector<FbxVector4> points = MyObjLoader(reconFiles[i]);
            if(i == 0){
                FbxVector4 revised_ori = Mesh->GetControlPoints()[i];
                printf("%f %f %f -> %f %f %f\n\n", revised_ori[0], revised_ori[1], revised_ori[2], points[i][0], points[i][1], points[i][2]);
            }
            
            for (int i = 0; i < vertsNum; i++) {
                newShape->GetControlPoints()[i] = points[i];
            }
            // add new channel to FbxBlendShape
            FbxBlendShapeChannel* NewChannel = FbxBlendShapeChannel::Create(lCompositeScene, bsName[i].c_str());
            BlendShape->AddBlendShapeChannel(NewChannel);

            NewChannel->AddTargetShape(newShape);
            NewChannel->DeformPercent.Set(0);

            
        }
    }

    numTabs = 0;
    GetNodeRecursive(lCompositeScene->GetRootNode(), &numTabs);

    if(lResult)
    {
        // Create an exporter.
        FbxExporter* lExporter = FbxExporter::Create(lSdkManager, "");

        // Initialize the exporter.
        lResult = lExporter->Initialize(outputFbxFile.c_str(), lFormat, lSdkManager->GetIOSettings());
        if (!lResult)
        {
            FBXSDK_printf("%s:\tCall to FbxExporter::Initialize() failed.\n", "FBX binary (*.fbx)");
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
    

    return 0;
}