#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * based on code from ProffieOS, copyright Fredrik Hubinette et al.
 *
 * proffieconstructs/vector3d.h
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

class Vector3D {
public:
    Vector3D();
    Vector3D(float);
    Vector3D(float, float, float);

    Vector3D operator+(const Vector3D&) const;
    Vector3D operator+(const float) const;
    Vector3D operator-(const Vector3D&) const;
    Vector3D operator-(const float) const;
    Vector3D operator-() const;
    Vector3D operator*(const Vector3D&) const;
    Vector3D operator*(const float) const;
    Vector3D operator/(const Vector3D&) const;
    Vector3D operator/(const float) const;

    Vector3D operator+=(const Vector3D&);
    Vector3D operator-=(const Vector3D&);
    Vector3D operator*=(const Vector3D&);
    Vector3D operator/=(const Vector3D&);

    float dot(const Vector3D&);
    Vector3D cross(const Vector3D&);

    // Length squared
    float length2() const;
    float length() const;

    static void rotate90(float&, float&);
    static void rotate180(float&, float&);

    // Rotate about the given axis
    void rotateX90();
    void rotateY90();
    void rotateZ90();

    void rotateX180();
    void rotateY180();
    void rotateZ180();

    Vector3D rotateX(float) const;
    Vector3D rotateY(float) const;
    Vector3D rotateZ(float) const;
    Vector3D rotate(float) const;

    Vector3D moveTowardsZero(float) const;

    float x;
    float y;
    float z;

};

