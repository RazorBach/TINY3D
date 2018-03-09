#ifndef HEADER_H
#define HEADER_H
#include <assert.h>
#include <windows.h>

typedef unsigned IUINT32;
const int  RENDER_STATE_WIREFRAME = 1;		// ��Ⱦ�߿�
const int  RENDER_STATE_TEXTURE = 2;		// ��Ⱦ����
const int  RENDER_STATE_COLOR = 4;		// ��Ⱦ��ɫ

//=====================================================================
// ��Ⱦ�豸
//=====================================================================
struct Device {
	Device(int mwidth, int mheight,void *fb) {
		init(mwidth, mheight, fb);
	}
	void init(int width, int height, void *fb) {
		this->width = width;
		this->height = height;
		// �豸��ʼ����fbΪ�ⲿ֡���棬�� NULL �������ⲿ֡���棨ÿ�� 4�ֽڶ��룩
		int need = sizeof(void*) * (height * 2 + 1024) + width * height * 8;
		char *ptr = (char*)malloc(need + 64);
		char *framebuf, *zbuf;
		int j;
		assert(ptr);
		framebuffer = (IUINT32**)ptr;
		zbuffer = (float**)(ptr + sizeof(void*) * height);
		ptr += sizeof(void*) * height * 2;
		texture = (IUINT32**)ptr;
		ptr += sizeof(void*) * 1024;
		framebuf = (char*)ptr;
		zbuf = (char*)ptr + width * height * 4;
		ptr += width * height * 8;
		if (fb != NULL) framebuf = (char*)fb;
		for (j = 0; j < height; j++) {
			//��תͼ��
			framebuffer[j] = (IUINT32*)(framebuf + width * 4 * (height - j - 1));
			zbuffer[j] = (float*)(zbuf + width * 4 * j);
			memset(zbuffer[j], 0, width * 4);
		}
		texture[0] = (IUINT32*)ptr;
		texture[1] = (IUINT32*)(ptr + 16);
		memset(this->texture[0], 0, 64);
		tex_width = 2;
		tex_height = 2;
		max_u = 1.0f;
		max_v = 1.0f;
		width = width;
		height = height;
		background = 0xc0c0c0;
		foreground = 0;
		//transform_init(&this->transform, width, height);
		this->render_state = RENDER_STATE_WIREFRAME;
	}
	void clearzbuffer() {
		memset(zbuffer, 0, width * height * sizeof(float));
	}

	// ����
	void device_pixel(int x, int y, IUINT32 color) {
		if (((IUINT32)x) < (IUINT32)width && ((IUINT32)y) < (IUINT32)height) {
			framebuffer[y][x] = color;
		}
	}

	// ��� framebuffer �� zbuffer
	void device_clear(int mode) {
		int y, x;
		for (y = 0; y <height; y++) {
			IUINT32 *dst = framebuffer[y];
			IUINT32 cc = (height - 1 - y) * 230 / (height - 1);
			cc = (cc << 16) | (cc << 8) | cc;
			if (mode == 0) cc = background;
			for (x = width; x > 0; dst++, x--) dst[0] = cc;
		}
		for (y = 0; y < height; y++) {
			float *dst = zbuffer[y];
			for (x = width; x > 0; dst++, x--) dst[0] = 0.0f;
		}
	}

	~Device() {
		if (framebuffer)
			free(framebuffer);
		framebuffer = NULL;
		zbuffer = NULL;
		texture = NULL;
	}
	int width;                  // ���ڿ��
	int height;                 // ���ڸ߶�
	IUINT32 **framebuffer;      // ���ػ��棺framebuffer[y] ����� y��
	float **zbuffer;            // ��Ȼ��棺zbuffer[y] Ϊ�� y��ָ��
	IUINT32 **texture;          // ����ͬ����ÿ������
	int tex_width;              // ������
	int tex_height;             // ����߶�
	float max_u;                // ��������ȣ�tex_width - 1
	float max_v;                // �������߶ȣ�tex_height - 1
	int render_state;           // ��Ⱦ״̬
	IUINT32 background;         // ������ɫ
	IUINT32 foreground;         // �߿���ɫ
private:
	Device(const Device&);
	Device& operator=(const Device&);
};
#endif // !HEADER_H
