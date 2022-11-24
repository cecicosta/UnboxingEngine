#include "BoundingBox.h"

#include <algorithm>

BoundingBox::BoundingBox(const Vector3f &first, const Vector3f &second)
: mFaces(36)
, mVertices(24)
, mFirst(first)
, mSecond(second)
, mPivot((first + second)/2){

    SetVertices(mFirst, mSecond);
    SetTriangles();
}

void BoundingBox::SetTriangles() {
    mFaces.resize(36);

    //Front
    mFaces[0] = 3;
    mFaces[1] = 2;
    mFaces[2] = 6;
    mFaces[3] = 6;
    mFaces[4] = 5;
    mFaces[5] = 3;

    //Back
    mFaces[6] = 0;
    mFaces[7] = 4;
    mFaces[8] = 7;
    mFaces[9] = 7;
    mFaces[10] = 1;
    mFaces[11] = 0;

    //Top
    mFaces[12] = 4;
    mFaces[13] = 5;
    mFaces[14] = 6;
    mFaces[15] = 6;
    mFaces[16] = 7;
    mFaces[17] = 4;

    //Bottom
    mFaces[18] = 0;
    mFaces[19] = 1;
    mFaces[20] = 2;
    mFaces[21] = 2;
    mFaces[22] = 3;
    mFaces[23] = 0;

    //Right
    mFaces[24] = 1;
    mFaces[25] = 7;
    mFaces[26] = 6;
    mFaces[27] = 6;
    mFaces[28] = 2;
    mFaces[29] = 1;

    //Left
    mFaces[30] = 0;
    mFaces[31] = 3;
    mFaces[32] = 5;
    mFaces[33] = 5;
    mFaces[34] = 4;
    mFaces[35] = 0;
}

void BoundingBox::SetVertices(const Vector3f &first, const Vector3f &second) {
    mVertices.resize(24);

    mVertices[0] = &first.x;
    mVertices[1] = &first.y;
    mVertices[2] = &first.z;

    mVertices[3] = &second.x;
    mVertices[4] = &first.y;
    mVertices[5] = &first.z;

    mVertices[6] = &second.x;
    mVertices[7] = &first.y;
    mVertices[8] = &second.z;

    mVertices[9] = &first.x;
    mVertices[10] = &first.y;
    mVertices[11] = &second.z;

    mVertices[12] = &first.x;
    mVertices[13] = &second.y;
    mVertices[14] = &first.z;

    mVertices[15] = &first.x;
    mVertices[16] = &second.y;
    mVertices[17] = &second.z;

    mVertices[18] = &second.x;
    mVertices[19] = &second.y;
    mVertices[20] = &second.z;

    mVertices[21] = &second.x;
    mVertices[22] = &second.y;
    mVertices[23] = &first.z;
}

BoundingBox::BoundingBox(const BoundingBox &other) : BoundingBox::BoundingBox(other.getFirst(), other.getSecond()) {}

Vector3f BoundingBox::getFirst() const {
    return mFirst;
}

Vector3f BoundingBox::getSecond() const {
    return mSecond;
}

Vector3f BoundingBox::getPivot() const {
    return mPivot;
}

std::vector<float> BoundingBox::GetVertices() const {
    std::vector<float> cpy(mVertices.size());
    std::transform(mVertices.begin(), mVertices.end(), cpy.begin(), [](const float* v){
        return *v;
    });
    return cpy;
}

std::vector<unsigned int> BoundingBox::GetTriangles() const {
    return {mFaces};
}

std::vector<unsigned int> BoundingBox::GetVertexIdList(EFace face) const {
    return { mFaces.begin() + static_cast<unsigned int>(face)*6, mFaces.begin() + static_cast<unsigned int>(face)*6 + 6 };
}

Vector3f BoundingBox::GetVertex(EFace face, int vIndex) {
    auto vIndexList = GetVertexIdList(face);
    return {*mVertices[vIndexList[vIndex]*3], *mVertices[(vIndexList[vIndex])*3 + 1], *mVertices[(vIndexList[vIndex])*3 + 2]};
}

BoundingBox& BoundingBox::operator=(const BoundingBox &other) {
    mFirst = other.mFirst;
    mSecond = other.mSecond;
    mPivot = ((mFirst + mSecond) / 2);

    SetVertices(mFirst, mSecond);
    SetTriangles();

    return *this;
}
