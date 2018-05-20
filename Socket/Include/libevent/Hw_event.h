#pragma once
#include <windows.h>

void SetMouse(POINT pos);
void SetMouse(int x, int y);
void LeftDown();
void LeftUp();
void RightDown();
void RightUp();
void LeftClick();
void RightClick();
void SetDelay(int time);
void Mouse_MoveTo(POINT start, POINT end);
void Drag(POINT start, POINT end);
 