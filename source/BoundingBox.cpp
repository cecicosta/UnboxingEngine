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

    mFaces[0] = 0;
    mFaces[1] = 1;
    mFaces[2] = 2;
    mFaces[3] = 2;
    mFaces[4] = 3;
    mFaces[5] = 0;

    mFaces[6] = 4;
    mFaces[7] = 5;
    mFaces[8] = 6;
    mFaces[9] = 6;
    mFaces[10] = 7;
    mFaces[11] = 4;

    mFaces[12] = 3;
    mFaces[13] = 2;
    mFaces[14] = 6;
    mFaces[15] = 6;
    mFaces[16] = 5;
    mFaces[17] = 3;

    mFaces[18] = 0;
    mFaces[19] = 4;
    mFaces[20] = 7;
    mFaces[21] = 7;
    mFaces[22] = 1;
    mFaces[23] = 0;

    mFaces[24] = 7;
    mFaces[25] = 6;
    mFaces[26] = 2;
    mFaces[27] = 2;
    mFaces[28] = 1;
    mFaces[29] = 7;

    mFaces[30] = 3;
    mFaces[31] = 5;
    mFaces[32] = 4;
    mFaces[33] = 4;
    mFaces[34] = 0;
    mFaces[35] = 3;
}

void BoundingBox::SetVertices(const Vector3f &first, const Vector3f &second) {
    mVertices.resize(24);

    mVertices[0] = &first.x;
    mVertices[1] = &first.y;
    mVertices[2] = &second.z;

    mVertices[3] = &second.x;
    mVertices[4] = &first.y;
    mVertices[5] = &second.z;

    mVertices[6] = &second.x;
    mVertices[7] = &second.y;
    mVertices[8] = &second.z;

    mVertices[9] = &first.x;
    mVertices[10] = &second.y;
    mVertices[11] = &second.z;

    mVertices[12] = &first.x;
    mVertices[13] = &first.y;
    mVertices[14] = &first.z;

    mVertices[15] = &first.x;
    mVertices[16] = &second.y;
    mVertices[17] = &first.z;

    mVertices[18] = &second.x;
    mVertices[19] = &second.y;
    mVertices[20] = &first.z;

    mVertices[21] = &second.x;
    mVertices[22] = &first.y;
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

BoundingBox& BoundingBox::operator=(const BoundingBox &other) {
    mFirst = other.mFirst;
    mSecond = other.mSecond;
    mPivot = ((mFirst + mSecond) / 2);

    SetVertices(mFirst, mSecond);
    SetTriangles();

    return *this;
}
