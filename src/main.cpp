#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <windows.h>

#define width  1280
#define height  720

static float g_time = 45.9431;
unsigned long dest[width * height];

struct vec3 {
  union { float v[3]; struct { float x, y, z; }; };
  vec3()                              { x = y = z = 0.0f; }
  vec3(float a)                       { x = y = z = a; }
  vec3(float a, float b, float c)     { x = a;    y = b;    z = c;  }
  vec3  operator +  (const vec3 &a)   { return vec3(x + a.x, y + a.y, z + a.z); }
  vec3  operator -  (const vec3 &a)   { return vec3(x - a.x, y - a.y, z - a.z); }
  vec3  operator *  (const vec3 &a)   { return vec3(x * a.x, y * a.y, z * a.z); }
  vec3  operator /  (const vec3 &a)   { return vec3(x / a.x, y / a.y, z / a.z); }
  vec3  operator +  (float a)         { return vec3(x + a,   y + a,   z + a); }
  vec3  operator -  (float a)         { return vec3(x - a,   y - a,   z - a); }
  vec3  operator *  (float a)         { return vec3(x * a,   y * a,   z * a); }
  vec3 &operator += (const vec3 &a)   { x += a.x; y += a.y; z += a.z; return *this; }
  vec3  operator >  (const vec3 &a)   { return vec3(x > a.x, y > a.y, z > a.z); }
  float dot(vec3 &a)                  { return x * a.x + y * a.y + z * a.z; }
  float length()                      { return sqrt(dot(*this));}
  vec3 &normalize()                   { float l = 1.0f / length(); return *this = vec3(x * l, y * l, z * l); }
  float frac(float a)                 { a -= (float)(int)a; if(a < 0) return -a; return a; }
  vec3  frac()                        { return vec3(frac(x), frac(y), frac(z)); }
  vec3  abs()                         { return vec3(::abs(x), ::abs(y), ::abs(z)); }
  vec3  integer()                     { return vec3((float)(int)(x), (float)(int)(y), (float)(int)(z)); }
  vec3 &left()                        { float t = x; x = y; y = z; z = t; return *this;}
  vec3 &right()                       { float t = z; z = y; y = x; x = t; return *this;}
  DWORD color()
  {
    float r = x > 1.0f ? 1.0f : x < 0.0f ? 0.0f : x;
    float g = y > 1.0f ? 1.0f : y < 0.0f ? 0.0f : y;
    float b = z > 1.0f ? 1.0f : z < 0.0f ? 0.0f : z;
    return (int(r * 255) << 16) | (int(g * 255) << 8) | int(b * 255);
  }
  void  disp()                        { printf("%f %f %f \n", x, y, z); }
};

union color {
  unsigned long argb;
  struct { unsigned char b, g, r, a; };
  color()                { argb = 0; }
  color(unsigned long c) { argb = c; }
  color &blend(color &c, int k) {
    int k1 = k;
    int k2 = 256 - k1;
    a = ((int(a) * k1) + (int(c.a) * k2)) >> 8;
    r = ((int(r) * k1) + (int(c.r) * k2)) >> 8;
    g = ((int(g) * k1) + (int(c.g) * k2)) >> 8;
    b = ((int(b) * k1) + (int(c.b) * k2)) >> 8;
    return *this;
  }
  
  color &blend(color &c, int k, int l, int m, int n) {
    int k1 = k; int k2 = 256 - k1;
    int l1 = l; int l2 = 256 - l1;
    int m1 = m; int m2 = 256 - m1;
    int n1 = n; int n2 = 256 - n1;
    a = ((int(a) * k1) + (int(c.a) * k2)) >> 8;
    r = ((int(r) * l1) + (int(c.r) * l2)) >> 8;
    g = ((int(g) * m1) + (int(c.g) * m2)) >> 8;
    b = ((int(b) * n1) + (int(c.b) * n2)) >> 8;
    return *this;
  }
};

struct param {
  float dist;
  vec3 n;
};

inline void map(param *p, vec3 pos, vec3 dir) {
  vec3 start = pos;
  vec3 t = pos;
  float d = 1.0f, n = 0.0f, Z = 3.0f;
  param prm;
  for(int i = 50; i-- && d > 0.003f; ) {
    d /= Z;
    t = t.frac() * Z;
    int j = int(t.integer().dot(t.integer())) % 4;
    if(j >= 2) {
      t    = (((dir > 0.0f) - t.frac()) / dir);
      n    = min(min(t.x, t.y), t.z);
      pos += (dir * (n * d + 0.001f));
      t    = pos;
      d    = 1;
    }
  }
  p->dist = (pos - start).length();
  p->n = n;
}

int main(int argc, char *argb) {
  memset(dest, 0, sizeof(dest));
  const int dens = 255;
  
  for(float l_time = g_time - 0.1; l_time < g_time; l_time += 0.003) {
    float a      = l_time, n = a * 0.1f;
    float v      = 0.14f;
    vec3 pos     = vec3(0.5f + sin(a * 0.2f) * v, 0.5f + cos(a * 0.3f) * v, 3.0f * a * v + sin(a * v - 0.1f)).left();
    float sn     = sin(n * 4.2), cn     = cos(n);
    float aspect = float(width)   / float(height);
    float dx     = aspect * (2.0f / float(width));
    float dy     =           2.0f / float(height);
    float sy     = -1.0f;
    for(int j = 0 ; j < height; j++) {
      float sx = -1.0f;
      for(int i = 0 ; i < width; i++) {
        vec3 dir = vec3(sx, sy, 1.0f).normalize();
        dir.right() = vec3(cn * dir.x + sn * dir.z, dir.y, cn * dir.z - sn * dir.x);
        dir.right();
        dir.right() = vec3(cn * dir.x + sn * dir.z, dir.y, cn * dir.z - sn * dir.x);
        param p;
        map(&p, pos, dir);
        unsigned long result = ((dir * 0.1f) + vec3(0.3f * p.n.x * p.dist + p.dist * 0.5f)).color();
        color a(dest[i + j * width]);
        color b(result);
        a.blend(b, 256, 240, 230, 210);
        dest[i + j * width] = a.argb;
        sx += dx;
      }
      sy += dy;
    }
  }
  
  //out
  FILE *fp = fopen("0.ppm", "w");
  fprintf(fp, "P3\n%d %d\n%d\n", width, height, dens);
  for(int j = 0 ; j < height; j++) {
    for(int i = 0 ; i < width; i++) {
      unsigned long col = dest[i + j * width];
      fprintf(fp, "%d %d %d\n", (col & 0x00FF0000) >> 16, (col & 0x0000FF00) >> 8, (col & 0x000000FF) >> 0);
    }
  }
  fclose(fp);
}

