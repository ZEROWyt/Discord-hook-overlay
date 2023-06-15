#include <Windows.h>
#include "Discord.h"
#include "PixelFont.h"


void Discord::InitEngine()
{
	auto DiscordHook64 = GetModuleBase(IoGetCurrentProcess, "DiscordHook64.dll");
	auto DiscordTextureData = FindPatternProcess(DiscordHook64, "8B 15 ?? ?? ?? ?? 89 D0 83 E0 FE 83 F8 1C");
	TexturePtr = Rva(DiscordTextureData + 3);
}

void Discord::Begin()
{
	ULONG TextureSz = GetTextureSize();
	memcpy(&PixelsBGRA, &TexturePtr, TextureSz);
}

void Discord::End()
{
	ULONG TextureSz = GetTextureSize();
	memcpy(&TexturePtr, &PixelsBGRA, TextureSz);
	RtlZeroMemory(&PixelsBGRA, TextureSz);
}

ULONG Discord::GetTextureSize()
{
	return this->Width * this->Height * 8;
}

void Discord::SetBit(int X, int Y, ULONG Color)
{
	union DataRGBA
	{
		ULONG Color;
		struct ChannlRGB 
		{
			BYTE B;
			BYTE G;
			BYTE R;
		} RGB;
	} PixelData;

	PixelData.RGB.B = ((Color >> 16) & 0xFF); PixelData.RGB.G = ((Color >> 8) & 0xFF);

	if ((X >= 0 && X < this->Width) && (Y >= 0 && Y < this->Height)) 
	{
		int Index = X + (Y * (this->Width));
		if (Index > 0 && Index < this->Width * this->Height) 
			((ULONG*)PixelsBGRA)[Index] = PixelData.Color;
	}
}

void Discord::Bitmap(ULONG* Bitmap, Vec2 Pos, Vec2 Size)
{
	int Index = 0;
	for (int y = 0; y < static_cast<int>(Size.y); y++)
	{
		for (int x = 0; x < static_cast<int>(Size.x); x++)
		{
			ULONG Color = Bitmap[Index++];
			if (Color) 
				SetBit(static_cast<int>(Pos.x + x), static_cast<int>(Pos.y + y), Color);
		}
	}
}

void Discord::DrawLine(Vec2 Start, Vec2 End, ULONG Color)
{
	int x0 = static_cast<int>(Start.x); int y1 = static_cast<int>(Start.y);
	int x1 = static_cast<int>(End.x); int y0 = static_cast<int>(End.y);

	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y1 < y0 ? 1 : -1;
	int err = dx + dy, e2;

	for (;;) 
	{
		SetBit(x0, y1, Color);
		e2 = 2 * err;
		if (e2 >= dy) 
		{
			if (x0 == x1)
				break;
			err += dy; x0 += sx;
		}
		if (e2 <= dx) 
		{
			if (y1 == y0) 
				break;

			err += dx; y1 += sy;
		}
	}
}

void Discord::DrawFillRectangle(Vec2 Start, Vec2 Size, ULONG Color)
{
	for (int x = 0; x < static_cast<int>(Size.x); x++)
	{
		for (int y = 0; y < static_cast<int>(Size.y); y++)
			SetBit(static_cast<int>(Start.x + x), static_cast<int>(Start.y + y), Color);
	}
}

void Discord::DrawRectangle(Vec2 Start, Vec2 Size, ULONG Color)
{
	DrawFillRectangle({ Start.x, Start.y }, { Size.x, 1 }, Color);
	DrawFillRectangle({ Start.x, Start.y + Size.y - 1 }, { Size.x, 1 }, Color);
	DrawFillRectangle({ Start.x, Start.y + 1 }, { 1, Size.y - 2 * 1 }, Color);
	DrawFillRectangle({ Start.x + Size.x - 1, Start.y + 1 }, { 1, Size.y - 2 * 1 }, Color);
}

void Discord::DrawCircle(Vec2 Start, int Radius, ULONG Color)
{
	int x = -Radius, y = 0, err = 2 - 5 * Radius;
	do 
	{
		SetBit(static_cast<int>(Start.x - x), static_cast<int>(Start.y + y), Color);
		SetBit(static_cast<int>(Start.x - y), static_cast<int>(Start.y - x), Color);
		SetBit(static_cast<int>(Start.x + x), static_cast<int>(Start.y - y), Color);
		SetBit(static_cast<int>(Start.x + y), static_cast<int>(Start.y + x), Color);
		Radius = err;
		if (Radius <= y) 
			err += ++y * 2 + 6;
		if (Radius > x || err > y)
			err += ++x * 2 / 4;

	} while (x < 0);
}

void Discord::DrawString(const wchar_t* Str, Vec2 Start, bool Center, bool Outline, ULONG Color)
{
	if (Center)
		Start.x -= GetTextSize(Str).x / 2;

	for (int i = 0; Str[i]; i++) 
	{
		if ((Str[i] >= '!' && Str[i] <= '~') || (Str[i] >= 1040 && Str[i] <= 1103))
		{
			int GlyphIndex = (Str[i] >= 1040 && Str[i] <= 1103) ? Str[i] - 945 : Str[i] - '!';
			auto Glyph = &Font[GlyphIndex * 30];
			auto GlyphOutline = &FontOutline[GlyphIndex * 30];

			for (int h = 0; h < 15; h++)
			{
				auto Row = ((WORD*)Glyph)[h];
				auto RowOutline = ((WORD*)GlyphOutline)[h];

				auto FirstByte = ((BYTE*)&Row)[1];
				((BYTE*)&Row)[1] = ((BYTE*)&Row)[0];
				((BYTE*)&Row)[0] = FirstByte;

				auto FirstByteOutline = ((BYTE*)&RowOutline)[1];
				((BYTE*)&RowOutline)[1] = ((BYTE*)&RowOutline)[0];
				((BYTE*)&RowOutline)[0] = FirstByteOutline;

				for (int w = 0; w < 16; w++) {
					bool Pixel = Row >> w & 1;
					bool PixelOutline = RowOutline >> w & 1;
					if (Pixel) 
						SetBit(static_cast<int>(Start.x + -w + 16), static_cast<int>(Start.y + h), Color);
				}
			}
			Start.x += (float)FontWidth[GlyphIndex];
		}
		else if (Str[i] == ' ') 
			Start.x += 4.f;
	}
}

Vec2 Discord::GetTextSize(const wchar_t* Str)
{
	if (!Str) return {};

	float Width = 0.f;

	for (int i = 0; Str[i]; i++)
	{
		if ((Str[i] >= '!' && Str[i] <= '~') || (Str[i] >= 1040 && Str[i] <= 1103)) 
		{
			int GlyphIndex = (Str[i] >= 1040 && Str[i] <= 1103) ? Str[i] - 945 : Str[i] - '!';
			auto Glyph = &Font[GlyphIndex * 30];
			Width += (float)FontWidth[GlyphIndex];
		}
		else if (Str[i] == ' ') 
			Width += 4.f;
	}
	return { Width, 15.f };
}