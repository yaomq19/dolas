#include <iostream>
#include "dolas_render_pipeline.h"
namespace Dolas
{
RenderPipeline::RenderPipeline()
{

}

RenderPipeline::~RenderPipeline()
{

}

bool RenderPipeline::initialize()
{
    initMainWindow();
    initDirect3D();
    return true;
}

bool RenderPipeline::initMainWindow()
{
    //WNDCLASS wc;
    //wc.style = CS_HREDRAW | CS_VREDRAW;
    //wc.lpfnWndProc = MainWndProc;
    //wc.cbClsExtra = 0;
    //wc.cbWndExtra = 0;
    //wc.hInstance = m_hAppInst;
    //wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    //wc.hCursor = LoadCursor(0, IDC_ARROW);
    //wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    //wc.lpszMenuName = 0;
    //wc.lpszClassName = L"D3DWndClassName";

    //if (!RegisterClass(&wc))
    //{
    //    MessageBox(0, L"RegisterClass Failed.", 0, 0);
    //    return false;
    //}

    //// Compute window rectangle dimensions based on requested client area dimensions.
    //RECT R = { 0, 0, m_ClientWidth, m_ClientHeight };
    //// AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    //int width = R.right - R.left;
    //int height = R.bottom - R.top;

    //m_hMainWnd = CreateWindow(L"D3DWndClassName", m_MainWndCaption.c_str(),
    //    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, m_hAppInst, nullptr);
    //if (!m_hMainWnd)
    //{
    //    MessageBox(0, L"CreateWindow Failed.", 0, 0);
    //    return false;
    //}

    //ShowWindow(m_hMainWnd, SW_SHOW);
    //UpdateWindow(m_hMainWnd);

    //return true;
    return true;
}

bool RenderPipeline::initDirect3D()
{
    return true;
}

void RenderPipeline::render(const Scene* const scene)
{
    // Do nothing
    std::cout << "rendering......" << std::endl;
}

}