#version 330 core

const float PI = 3.141592654;
const float AMBIENT = 0.07;
const float LENGHT = 1;

in vec2 texCoord;
uniform float frame;

out vec4 fragColor;

vec4 quat(vec3 a, float angle) {
    float halfAngle = angle / 2;
    return vec4(a * sin(halfAngle), cos(halfAngle));
}

vec4 quatInv(vec4 q) {
    return vec4(-q.xyz, q.w);
}

vec4 quatMul(vec4 q1, vec4 q2) {
    vec3 vec3q1 = q1.xyz;
    vec3 vec3q2 = q2.xyz;
    return vec4(q1.w * vec3q2 + q2.w * vec3q1 + cross(vec3q1, vec3q2), q1.w * q2.w - dot(vec3q1, vec3q2));
}

vec3 quatRot(vec4 q, vec3 p) {
    vec4 qInv = quatInv(q);
    return quatMul(quatMul(q, vec4(p, 0)), qInv).xyz;
}

float intersectSphere(vec3 origin, vec3 rayDir, vec3 center, float r, out vec3 normal) {
    vec3 line = origin - center;
    float a = dot(rayDir, rayDir);
    float b = dot(line, rayDir) * 2.0;
    float c = dot(line, line) - pow(r, 2.0);
    float disc = pow(b,2.0) - 4.0 * a * c;
    if (disc < 0.0) {
        return -1.0;
    }
    float t = (-b - sqrt(pow(b,2.0) - 4.0 * a * c)) / (2.0 * a);
    vec3 hitPos = origin + rayDir * t;
    
    normal = normalize(hitPos - center);
    return t;
}

vec3 lampPoint = vec3(0,0,0);

float intersectPara(vec3 origin, vec3 rayDir, vec3 center, float height, out vec3 normal) {
    vec2 oc = origin.xz - center.xz;
    float a = dot(rayDir.xz, rayDir.xz);
    float b = 2.0 * dot(oc.x, rayDir.x) + 2.0 * dot(oc.y, rayDir.z) - rayDir.y;
    float c = dot(oc.x, oc.x) + dot(oc.y, oc.y) - (origin.y - center.y);
    float disc = b * b - 4 * a * c;
    if (disc < 0.0) {
        return -1.0;
    }
    float t1 = (-b - sqrt(disc)) / (2 * a);
    float t2 = (-b + sqrt(disc)) / (2 * a);
    vec3 hitPos1 = origin + rayDir * t1;
    vec3 hitPos2 = origin + rayDir * t2;
    if (hitPos1.y < center.y || hitPos1.y > height + center.y)
    t1 = -1.0;
    if (hitPos2.y < center.y || hitPos2.y > height + center.y)
    t2 = -1.0;

    float t;
    vec3 hitPos;
    if (t1 < 0.0 && t2 < 0.0) {
        t = -1.0;
    } else if (t2 < 0.0) {
        t = t1;
        hitPos = hitPos1;
    } else if (t1 < 0.0) {
        t = t2;
        hitPos = hitPos2;
    } else {
        if (t1 < t2) {
            t = t1;
            hitPos = hitPos1;
        } else {
            t = t2;
            hitPos = hitPos2;
        }
    }

    vec3 fx = vec3(1,2*(oc.x + rayDir.x * t),0);
    vec3 fz = vec3(0,2*(oc.y + rayDir.z * t),1);

    normal = cross(fx, fz);
    normal = normalize(normal);

    lampPoint = center + vec3(0,1,0);

    return t;
}


float intersectCylinder(vec3 origin, vec3 rayDir, vec3 center, float radius, float height, out vec3 normal) {
    vec2 oc = origin.xz - center.xz;
    float a = dot(rayDir.xz, rayDir.xz);
    float b = 2.0 * dot(oc, rayDir.xz);
    float c = dot(oc, oc) - radius * radius;
    float disc = b * b - 4 * a * c;
    if (disc < 0.0) {
        return -1.0;
    }
    float t1 = (-b - sqrt(disc)) / (2 * a);
    float t2 = (-b + sqrt(disc)) / (2 * a);
    vec3 hitPos1 = origin + rayDir * t1;
    vec3 hitPos2 = origin + rayDir * t2;
    if (hitPos1.y < center.y || hitPos1.y > height + center.y)
        t1 = -1.0;
    if (hitPos2.y < center.y || hitPos2.y > height + center.y)
        t2 = -1.0;
    
    float t;
    vec3 hitPos;
    if (t1 < 0.0 && t2 < 0.0) {
        t = -1.0;
    } else if (t2 < 0.0) {
        t = t1;
        hitPos = hitPos1;
    } else if (t1 < 0.0) {
        t = t2;
        hitPos = hitPos2;
    } else {
        if (t1 < t2) {
            t = t1;
            hitPos = hitPos1;
        } else {
            t = t2;
            hitPos = hitPos2;
        }
    }
    
    normal = hitPos - center;
    normal.y = 0.0;
    normal = normalize(normal);
        
    return t;
}

float intersectPlane(vec3 origin, vec3 rayDir, vec3 point, vec3 normal) {
    return dot(point - origin, normal) / dot(rayDir, normal);
}

float intersectCPlane(vec3 origin, vec3 rayDir, vec3 point, float r, vec3 normal) {
    
    float t = dot(point - origin, normal) / dot(rayDir, normal);
    vec3 target = origin + rayDir * t;
    if(dot(target - point, target - point) <= r * r)
    {
        return t;
    }
    return -1.0;
}

float combine(float t1, float t2, vec3 normal1, vec3 normal2, out vec3 normal) {
    float t;
    if (t1 < 0.0 && t2 < 0.0) {
        return -1.0;
    } else if (t2 < 0.0) {
        normal = normal1;
        return t1;
    } else if (t1 < 0.0) {
        normal = normal2;
        return t2;
    } else {
        if (t1 < t2) {
            normal = normal1;
            return t1;
        } else {
            normal = normal2;
            return t2;
        }
    }
} 



float intersectWorld(vec3 origin, vec3 rayDir, out vec3 normal) {
    float time = frame / 60.0;
    lampPoint = vec3(0,0,0);
    
    //Forgas
    vec4 q = quat(normalize(vec3(0, 1, 0)), time);
    vec3 rotOrigin = quatRot(q, origin);
    vec3 rotRayDir = quatRot(q, rayDir);

    //Asztal
    vec3 planeNormal = vec3(0, 1, 0);
    float tPlane = intersectPlane(origin, rayDir, vec3(0, -0.2, 0), planeNormal);
    
    //Talp
    vec3 cyl1Normal;
    float tCy1l = intersectCylinder(rotOrigin, rotRayDir, vec3(0, -0.2, 0), 1.2, 0.2, cyl1Normal);
    cyl1Normal = quatRot(quatInv(q), cyl1Normal);

    //Talp teteje
    vec3 plane2Normal = vec3(0,1,0);
    float tPlane2 = intersectCPlane(rotOrigin, rotRayDir, vec3(0, 0, 0), 1.2, plane2Normal);

    //1. Gomb
    vec3 sphNormal;
    float tSph = intersectSphere(origin, rayDir, vec3(0, 0, 0), 0.15, sphNormal);

    //Finished 1. Rud
    vec3 rotLineSegment1 = vec3(0,3,1);
    vec4 qSegment1 = quat(normalize(rotLineSegment1), time * 4);
    vec3 rotOriginSegment1 = quatRot(qSegment1, rotOrigin);
    vec3 rotRayDirSegment1 = quatRot(qSegment1, rotRayDir);
    vec3 cyl3Normal;
    float tCy3l = intersectCylinder(rotOriginSegment1, rotRayDirSegment1, vec3(0, 0, 0), 0.05, 1.5, cyl3Normal);
    cyl3Normal = quatRot(quatInv(qSegment1), cyl3Normal);
    cyl3Normal = quatRot(quatInv(q), cyl3Normal);

    //Finished 2. Gomb
    vec3 sph4Normal;
    float tSph4 = intersectSphere(rotOriginSegment1, rotRayDirSegment1, vec3(0, 1.5, 0), 0.15, sph4Normal);
    sph4Normal = quatRot(quatInv(qSegment1), sph4Normal);
    sph4Normal = quatRot(quatInv(q), sph4Normal);

    //Finished 2. Rud
    vec4 qSegment2 = quat(normalize(vec3(0,1,1)), time * 4);
    vec3 rotOriginSegment2 = quatRot(qSegment2, rotOriginSegment1 + vec3(0,-1.5,0));
    vec3 rotRayDirSegment2 = quatRot(qSegment2, rotRayDirSegment1);
    vec3 cyl5Normal;
    float tCy5l = intersectCylinder(rotOriginSegment2, rotRayDirSegment2, vec3(0, 0, 0), 0.05, 1.5, cyl5Normal);
    cyl5Normal = quatRot(quatInv(qSegment2), cyl5Normal);
    cyl5Normal = quatRot(quatInv(qSegment1), cyl5Normal);
    cyl5Normal = quatRot(quatInv(q), cyl5Normal);

    //Teszt 3. Gomb
    vec3 sph2Normal;
    float tSph2 = intersectSphere(rotOriginSegment2, rotRayDirSegment2, vec3(0, 1.5, 0), 0.2, sph2Normal);
    sph2Normal = quatRot(quatInv(qSegment2), sph2Normal);
    sph2Normal = quatRot(quatInv(qSegment1), sph2Normal);
    sph2Normal = quatRot(quatInv(q), sph2Normal);


    //Old Para

    vec4 qSegment3 = quat(normalize(vec3(0,4,1)), time * 4);
    vec3 rotOriginSegment3 = quatRot(qSegment3, rotOriginSegment2 + vec3(0,-1.5,0));
    vec3 rotRayDirSegment3 = quatRot(qSegment3, rotRayDirSegment2);
    vec3 parNormal;
    float tPar = intersectPara(rotOriginSegment3, rotRayDirSegment3, vec3(0, 0, 0), 0.55, parNormal);
    parNormal = quatRot(quatInv(qSegment3), parNormal);
    parNormal = quatRot(quatInv(qSegment2), parNormal);
    parNormal = quatRot(quatInv(qSegment1), parNormal);
    parNormal = quatRot(quatInv(q), parNormal);


    lampPoint = quatRot(quatInv(qSegment3), lampPoint);
    
    float t = combine(tPlane2, tSph, plane2Normal, sphNormal, normal);
    t = combine(t, tCy1l, normal, cyl1Normal, normal);
    t = combine(t, tSph2, normal, sph2Normal, normal);
    t = combine(t, tCy3l, normal, cyl3Normal, normal); //Teszt Rud
    t = combine(t, tSph4, normal, sph4Normal, normal); //Teszt Gomb
    t = combine(t, tCy5l, normal, cyl5Normal, normal); //Teszt Rud2
    t = combine(t, tPar, normal, parNormal, normal);

    return combine(t, tPlane, normal, planeNormal, normal);
}

void main() {
    float time = frame / 60.0;
    vec3 lightPos = vec3(cos(time) * 5.0, 7, sin(time) * 5.0);
    //vec3 light2Pos = vec3(cos(time) * 5.0, 7, sin(time) * 5.0);
    vec3 light2Pos;
    
    float fov = PI / 2;
    
    vec3 origin = vec3(0, 2, 5);
    vec3 rayDir = normalize(vec3(texCoord * 2 - 1, -tan(fov / 2.0)));
    //rayDir = (vec4(rayDir, 0) * viewMat).xyz;
    
    vec3 normal;
    float t = intersectWorld(origin, rayDir, normal);
    
    if (dot(normal, rayDir) > 0.0) {
        normal *= -1;
    }
    
    vec3 hitPos = origin + rayDir * t;

    vec3 toLight = lightPos - hitPos;
    float distToLight = length(toLight);
    toLight /= distToLight;
    if (t > 0.0) {
        float cosTheta = max(dot(toLight, normal), 0.0);
        vec3 _;
        float lightT = intersectWorld(hitPos + normal * 0.0001, toLight, _);
        float lightIntensity = 40.0;
        if (lightT >= 0.0) {
            lightIntensity = 0.0;
        }
        else
        {
            fragColor = vec4(0, 0, 0, 1);
        }
        fragColor += vec4(vec3(1.0, 0.0, 1.0) * cosTheta / pow(distToLight, 2.0) * lightIntensity + AMBIENT, 1);
    }
    //lampPoint = vec3(1,1,1);
    vec3 toLight2 = lampPoint - hitPos;
    float distToLight2 = length(toLight2);
    toLight2 /= distToLight2;
    if (t > 0.0) {
        float cosTheta2 = max(dot(toLight2, normal), 0.0);
        vec3 _;
        float light2T = intersectWorld(hitPos + normal * 0.0001, toLight2, _);
        float light2Intensity = 40.0;
        if (light2T >= 0.0) {
            light2Intensity = 0.0;
        }
        
        //fragColor += vec4(vec3(1.0, 0, 1.0) * cosTheta2 / pow(distToLight2, 2.0) * light2Intensity + AMBIENT, 1);
    } else {
        //fragColor = vec4(0, 0, 0, 1);
    }
    
}