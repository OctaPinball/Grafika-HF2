//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Scholtz Bálint András
// Neptun : A8O5M2
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================

//Kiindulási alapként használtam Csala Bálint grafika konzultációs felvételét több függvénynél.
#include "framework.h"



const char* const vertexSource = R"(
#version 330 
in vec4 vertexPosition; 
out vec2 texCoord; 
void main() { 
    gl_Position = vertexPosition; 
    texCoord = vertexPosition.xy * 0.5 + 0.5; 
}
)";

const char* const fragmentSource = R"(
#version 330 core

const float PI = 3.14159265359;
const float AMBIENT = 0.07;
const float SHININESS = 80;

in vec2 texCoord;
uniform float frame;

out vec4 fragColor;

struct Ray{
    vec3 position, direction;
};

struct Sphere{
    vec3 position;
    float radius;
    vec3 normal;
};

struct Cylinder{
    vec3 position;
    float radius, height;
    vec3 normal;
};

struct Paraboloid{
    vec3 position;
    float height;
    vec3 normal;
};

struct Plane{
    vec3 position;
    vec3 normal;
};

struct CirclePlane{
    vec3 position;
    vec3 normal;
    float radius;
};

struct Light{
    Ray ray;
    vec3 lightpos;
};

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

vec3 getHitPos(Ray r, float t){
    return r.position + r.direction * t;
}


float hitSphere(Ray ray, out Sphere sphere) {
    vec3 line = ray.position - sphere.position;
    float a = dot(ray.direction, ray.direction);
    float b = dot(line, ray.direction) * 2.0;
    float c = dot(line, line) - pow(sphere.radius, 2.0);
    float disc = pow(b,2.0) - 4.0 * a * c;
    if (disc < 0.0) {
        return -1.0;
    }
    float t = (-b - sqrt(pow(b,2.0) - 4.0 * a * c)) / (2.0 * a);
    vec3 hitPos = getHitPos(ray, t);
    
    sphere.normal = normalize(hitPos - sphere.position);
    return t;
}

vec3 lampPoint = vec3(0,0,0);


float hitParaboloid(Ray ray, out Paraboloid para) {
    vec2 oc = ray.position.xz - para.position.xz;
    float a = dot(ray.direction.xz, ray.direction.xz);
    float b = 2.0 * dot(oc.x, ray.direction.x) + 2.0 * dot(oc.y, ray.direction.z) - ray.direction.y;
    float c = dot(oc.x, oc.x) + dot(oc.y, oc.y) - (ray.position.y - para.position.y);
    float disc = b * b - 4 * a * c;
    if (disc < 0.0) {
        return -1.0;
    }
    float t1 = (-b - sqrt(disc)) / (2 * a);
    float t2 = (-b + sqrt(disc)) / (2 * a);
    vec3 hitPos1 = getHitPos(ray, t1);
    vec3 hitPos2 = getHitPos(ray, t2);
    if (hitPos1.y < para.position.y || hitPos1.y > para.height + para.position.y)
    t1 = -1.0;
    if (hitPos2.y < para.position.y || hitPos2.y > para.height + para.position.y)
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

    vec3 fx = vec3(1,2*(oc.x + ray.direction.x * t),0);
    vec3 fz = vec3(0,2*(oc.y + ray.direction.z * t),1);

    para.normal = cross(fx, fz);
    para.normal = normalize(para.normal);

    //lampPoint = para.position + vec3(0,1,0);

    return t;
}


float hitCylinder(Ray ray, out Cylinder cyl) {
    vec2 oc = ray.position.xz - cyl.position.xz;
    float a = dot(ray.direction.xz, ray.direction.xz);
    float b = 2.0 * dot(oc, ray.direction.xz);
    float c = dot(oc, oc) - pow(cyl.radius, 2.0);
    float disc = b * b - 4 * a * c;
    if (disc < 0.0) {
        return -1.0;
    }
    float t1 = (-b - sqrt(disc)) / (2 * a);
    float t2 = (-b + sqrt(disc)) / (2 * a);
    vec3 hitPos1 = getHitPos(ray, t1);
    vec3 hitPos2 = getHitPos(ray, t2);
    if (hitPos1.y < cyl.position.y || hitPos1.y > cyl.height + cyl.position.y)
        t1 = -1.0;
    if (hitPos2.y < cyl.position.y || hitPos2.y > cyl.height + cyl.position.y)
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
    
    cyl.normal = hitPos - cyl.position;
    cyl.normal.y = 0.0;
    cyl.normal = normalize(cyl.normal);
        
    return t;
}

float hitPlane(Ray ray, Plane plane) {
    return dot(plane.position - ray.position, plane.normal) / dot(ray.direction, plane.normal);
}


float hitCircPlane(Ray ray, CirclePlane CP) {
    
    float t = dot(CP.position - ray.position, CP.normal) / dot(ray.direction, CP.normal);
    vec3 target = ray.position + ray.direction * t;
    if(dot(target - CP.position, target - CP.position) <= pow(CP.radius, 2.0))
    {
        return t;
    }
    return -1.0;
}


float merge(float t1, float t2, vec3 normal1, vec3 normal2, out vec3 normal) {
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

Ray rotateRay(Ray ray, vec4 quat){
    Ray r;
    r.position = quatRot(quat, ray.position);
    r.direction = quatRot(quat, ray.direction);
    return r;
}

Ray rotateRay(Ray ray, vec4 quat, vec3 offset){
    Ray r;
    r.position = quatRot(quat, ray.position + offset);
    r.direction = quatRot(quat, ray.direction);
    return r;
}

float hitScene(Ray ray, out vec3 normal) {
    float time = frame / 60.0 + 19;
    lampPoint = vec3(0,4,0);
    
    vec4 q = quat(normalize(vec3(0, 4, 0)), time);
    Ray rayCamera = rotateRay(ray, q);

    Plane plane = Plane(vec3(0,-0.2,0), vec3(0,1,0));
    float tPlane = hitPlane(ray, plane);
    
    Cylinder Cyl1 = Cylinder(vec3(0, -0.2, 0), 1.2, 0.2, vec3(0,0,0));
    float tCyl1 = hitCylinder(rayCamera, Cyl1);
    Cyl1.normal = quatRot(quatInv(q), Cyl1.normal);

    CirclePlane CP = CirclePlane(vec3(0,0,0),vec3(0,1,0), 1.2);
    float tCP = hitCircPlane(rayCamera, CP);

    Sphere sph1 = Sphere(vec3(0,0,0), 0.15, vec3(0,0,0));
    float tSph1 = hitSphere(ray, sph1);

    vec4 qSegment1 = quat(normalize(vec3(0,3,1)), time * 4);
    Ray raySegment1 = rotateRay(rayCamera, qSegment1);
    Cylinder Cyl2 = Cylinder(vec3(0,0,0), 0.05, 1.5, vec3(0,0,0));
    float tCyl2 = hitCylinder(raySegment1, Cyl2);
    Cyl2.normal = quatRot(quatInv(qSegment1), Cyl2.normal);
    Cyl2.normal = quatRot(quatInv(q), Cyl2.normal);

    Sphere sph2 = Sphere(vec3(0,1.5,0), 0.15, vec3(0,0,0));
    float tSph2 = hitSphere(raySegment1, sph2);
    sph2.normal = quatRot(quatInv(qSegment1), sph2.normal);
    sph2.normal = quatRot(quatInv(q), sph2.normal);

    vec4 qSegment2 = quat(normalize(vec3(0,1,1)), time * 4);
    Ray raySegment2 = rotateRay(raySegment1, qSegment2, vec3(0,-1.5,0));
    Cylinder Cyl3 = Cylinder(vec3(0,0,0), 0.05, 1.5, vec3(0,0,0));
    float tCyl3 = hitCylinder(raySegment2, Cyl3);
    Cyl3.normal = quatRot(quatInv(qSegment2), Cyl3.normal);
    Cyl3.normal = quatRot(quatInv(qSegment1), Cyl3.normal);
    Cyl3.normal = quatRot(quatInv(q), Cyl3.normal);

    Sphere sph3 = Sphere(vec3(0,1.5,0),0.2,vec3(0,0,0));
    float tSph3 = hitSphere(raySegment2, sph3);
    sph3.normal = quatRot(quatInv(qSegment2), sph3.normal);
    sph3.normal = quatRot(quatInv(qSegment1), sph3.normal);
    sph3.normal = quatRot(quatInv(q), sph3.normal);

    vec4 qSegment3 = quat(normalize(vec3(0,4,1)), time * 4);
    Ray raySegment3 = rotateRay(raySegment2, qSegment3, vec3(0,-1.5,0));
    Paraboloid para = Paraboloid(vec3(0,0,0), 0.55, vec3(0,0,0));
    float tPara = hitParaboloid(raySegment3, para);
    para.normal = quatRot(quatInv(qSegment3), para.normal);
    para.normal = quatRot(quatInv(qSegment2), para.normal);
    para.normal = quatRot(quatInv(qSegment1), para.normal);
    para.normal = quatRot(quatInv(q), para.normal);


    lampPoint = quatRot(quatInv(qSegment3), lampPoint);

    lampPoint = vec3(0,0,0);
    lampPoint += Cyl1.normal;
    lampPoint += Cyl2.normal;
    lampPoint += Cyl3.normal;
    lampPoint += para.normal;
    
    float t = merge(tCP, tSph1, CP.normal, sph1.normal, normal);
    t = merge(t, tCyl1, normal, Cyl1.normal, normal);
    t = merge(t, tSph2, normal, sph2.normal, normal);
    t = merge(t, tCyl3, normal, Cyl3.normal, normal);
    t = merge(t, tSph2, normal, sph2.normal, normal);
    t = merge(t, tCyl2, normal, Cyl2.normal, normal);
    t = merge(t, tPara, normal, para.normal, normal);
    t = merge(t, tPlane, normal, plane.normal, normal);

    return t;
}



vec4 glow(Light light, float t, vec3 normal){
    vec4 colorOut = vec4(0,0,0,0);
    vec3 hitPos = getHitPos(light.ray, t);
    vec3 toLight = light.lightpos - hitPos;
    float distToLight = length(toLight);
    toLight /= distToLight;
    if (t > 0.0) {
        float cosTheta = max(dot(toLight, normal), 0.0);
        vec3 halfway = normalize(-light.ray.direction + toLight + vec3(0,0,0.5));
        float cosDelta = dot(normal, halfway);
        vec3 _;
        Ray lightRay = Ray(hitPos + normal * 0.1, toLight);
        float lightT = hitScene(lightRay, _);
        float lightIntensity = 40.0;
        if (lightT >= 0.0) {
            lightIntensity = 0.0;
        }
        else
        {
            colorOut = vec4(0, 0, 0, 1);
        }
        colorOut = vec4((vec3(0.0, 1.0, 1.0) * cosTheta / pow(distToLight, 2.0) ) * lightIntensity + AMBIENT, 1);
        colorOut += vec4(vec3(1.0, 1.0, 1.0) * pow(cosDelta, SHININESS) * lightIntensity, 1);
    }
    return colorOut;
}

void main() {
    float time = frame / 60.0 + 19;
    vec3 lightPos = vec3(cos(time) * 5.0, 7, sin(time) * 5.0);
    //vec3 light2Pos = vec3(cos(time) * 5.0, 7, sin(time) * 5.0);
    vec3 light2Pos;

    float fov = PI / 2;
    Ray ray = Ray(vec3(0, 2, 5), normalize(vec3(texCoord * 2 - 1, -tan(fov / 2.0))));
    
    vec3 normal;
    float t = hitScene(ray, normal);
    
    if (dot(normal, ray.direction) > 0.0) {
        normal *= -1;
    }

    fragColor = glow(Light(ray, lightPos), t, normal);
    //fragColor += glow(Light(ray, lampPoint), t, normal);
   
}
)";

GPUProgram gpuProgram(false);
unsigned int vao;
int frame = 0;

void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    float vertices[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    gpuProgram.create(vertexSource, fragmentSource, "outColor");
}


void onDisplay() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);


    gpuProgram.setUniform((float)frame, "frame");
    frame++;

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glutSwapBuffers();
    glutPostRedisplay();
    glDeleteBuffers;
}

void onKeyboard(unsigned char key, int pX, int pY) {
}

void onKeyboardUp(unsigned char key, int pX, int pY) {
}

void onMouseMotion(int pX, int pY) {
}

void onMouse(int button, int state, int pX, int pY) {
}

void onIdle() {
    glutPostRedisplay();
}
