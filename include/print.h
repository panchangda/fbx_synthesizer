#pragma once
void GetNodeRecursive(FbxNode* lRootNode, int* numTabs);
void PrintNode(FbxNode* pNode, int* numTabs);
void PrintAttribute(FbxNodeAttribute* pAttribute, int* numTabs);
FbxString GetAttributeTypeName(FbxNodeAttribute::EType type);
void PrintTabs(int numTabs);
