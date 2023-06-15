#pragma once

struct Vec2
{
	float x;
	float y;
};

class Discord
{
public:
	void InitEngine();
	void Begin();
	void End();
	void SetBit(int X, int Y, ULONG Color);
	void Bitmap(ULONG* Bitmap, Vec2 Pos, Vec2 Size);
	void DrawLine(Vec2 Start, Vec2 End, ULONG Color);
	void DrawCircle(Vec2 Start, int Radius, ULONG Color);
	void DrawRectangle(Vec2 Start, Vec2 Size, ULONG Color);
	void DrawFillRectangle(Vec2 Start, Vec2 Size, ULONG Color);
	void DrawString(const wchar_t* Str, Vec2 Start, bool Center, bool Outline, ULONG Color);
	Vec2 GetTextSize(const wchar_t* Str);
	ULONG GetTextureSize();
private:
	ULONG64 TexturePtr;
	ULONG Width;
	ULONG Height;
	ULONG PixelsBGRA[];
};

