#include "DXCommon.h"

#include <cassert>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#include "Logger.h"
#include "StringUtility.h"
#include <format>
#include <thread>


const float DXCommon::kDeltaTime=1.0f/60.0f;
std::unique_ptr<DXCommon> DXCommon::instance=nullptr;

DXCommon* DXCommon::GetInstance() {
    if (instance == nullptr) {
        // コンストラクタがprivateなのでmake_uniqueではなくnewしてresetする
        instance.reset(new DXCommon());
    }
    return instance.get();
}// 静的メンバ変数の初期化
void DXCommon::Initialize()
{
    InitializeFixFPS();
    CreateDevice();
    CreateCommand();
    CreateSwapChain();
    CreateDepthStencilTextureResource();
    CreateDescriptorHeaps();
    CreateRenderTargetView();
    CreateDepthStencilView();
    CreateFence();
    CreateViewport();
    CreateScissorRect();
    CreateDXCompiler();
    
}

void DXCommon::Finalize()
{
   instance.reset();
}

void DXCommon::PreDraw()
{
    //バックバッファのインデックス取得
    UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();
    //リソースバリアで書き込み可能に変更
    barrier_ = {};
    //Transitionバリアー
    barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    //noneにする
    barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    //バリアを得るリソース。バックアップｂufferのインデックスを取得
    barrier_.Transition.pResource = swapChainResources_[backBufferIndex].Get();
    //遷移前（現在）のリソース状態
    barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    //遷移後のリソース状態
    barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    //transitionバリアーを張る
    commandList_->ResourceBarrier(1, &barrier_);
    //描画先のRTVとDSVの設定
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetCPUDescriptorHandle(dsvHeap_, descriptorSizeDSV_, 0);
    commandList_->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex], FALSE, &dsvHandle);
    //画面クリア
      //クリアカラー
    float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
    //
    commandList_->ClearRenderTargetView(
        rtvHandles_[backBufferIndex],
        clearColor,
        0,
        nullptr
    );
    //画面深度クリア
    commandList_->ClearDepthStencilView(
        dsvHandle,
        D3D12_CLEAR_FLAG_DEPTH,
        1.0f, 0, 0, nullptr
    );
    //SRVヒープの設定
   // ID3D12DescriptorHeap* descriptorHeaps[] = { srvHeap_.Get() };
   // commandList_->SetDescriptorHeaps(1, descriptorHeaps);
    //ビューポート・シザー矩形の設定
    commandList_->RSSetViewports(1, &viewport_);//ビューポートの設定
    commandList_->RSSetScissorRects(1, &scissorRect_);//シザー矩形の設定
}

void DXCommon::PostDraw()
{
    //バックバッファのインデックス取得
    UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

    barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier_.Transition.pResource = swapChainResources_[backBufferIndex].Get();
    barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    //リソースバリアでプレゼント可能に変更
    commandList_->ResourceBarrier(1, &barrier_);
    //コマンドリストのクローズ
    hr_ = commandList_->Close();
    assert(SUCCEEDED(hr_));
    //コマンドリストの実行
    ID3D12CommandList* commandLists[] = { commandList_.Get() };
    commandQueue_->ExecuteCommandLists(1, commandLists);
    //画面に表示する
    hr_ = swapChain_->Present(1, 0);
    assert(SUCCEEDED(hr_));
    //次のフレームへ
    fenceValue_++;
    commandQueue_->Signal(fence_.Get(), fenceValue_);
    //待機
    //現在のフェンス値がゴール値に到達しているか確認
    if (fence_.Get()->GetCompletedValue() < fenceValue_)
    {
        HANDLE fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
        assert(fenceEvent_ != nullptr);
        fence_.Get()->SetEventOnCompletion(fenceValue_, fenceEvent_);
        WaitForSingleObject(fenceEvent_, INFINITE);
        CloseHandle(fenceEvent_);
    }
    //コマンドアロケーターのリセット
    hr_ = commandAllocator_->Reset();
    assert(SUCCEEDED(hr_));
    //コマンドリストのリセット
    hr_ = commandList_->Reset(commandAllocator_.Get(), nullptr);
    assert(SUCCEEDED(hr_));

}


//D3D12_CPU_DESCRIPTOR_HANDLE DXCommon::GetSRVCPUDescriptorHandle(uint32_t index)
//{
//    return GetCPUDescriptorHandle(srvHeap_, descriptorSizeSRV_, index);
//}
//
//D3D12_GPU_DESCRIPTOR_HANDLE DXCommon::GetSRVGPUDescriptorHandle(uint32_t index)
//{
//    return GetGPUDescriptorHandle(srvHeap_, descriptorSizeSRV_, index);
//}

Microsoft::WRL::ComPtr<IDxcBlob> DXCommon::CompileShader(const std::wstring& filePath, const wchar_t* profile)
{
    //hlslファイルの読み込み
    Logger::Log(StringUtility::ConvertString(std::format(L"Bigin CompileShader, path:{},profiale:{}\n", filePath, profile)));
    IDxcBlobEncoding* shaderSource = nullptr;
    hr_ = dxcUtils->LoadFile(
        filePath.c_str(),
        nullptr,
        &shaderSource
    );
    assert(SUCCEEDED(hr_));
    DxcBuffer shaderSoursBuffer{};
    shaderSoursBuffer.Ptr = shaderSource->GetBufferPointer();
    shaderSoursBuffer.Size = shaderSource->GetBufferSize();
    shaderSoursBuffer.Encoding = DXC_CP_ACP;
    //compileする
    LPCWSTR arguments[] = {
        filePath.c_str(),
        L"-E",L"main",
        L"-T",profile,
        L"-Zi",L"-Qembed_debug",
        L"-Od",
        L"-Zpr",

    };

    Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
    hr_ = dxcCompiler->Compile(
        &shaderSoursBuffer,
        arguments,
        _countof(arguments),
        includeHandler.Get(),
        IID_PPV_ARGS(&shaderResult)
    );
    assert(SUCCEEDED(hr_));

    //警告・エラー確認
    Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError = nullptr;
    shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
    if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
        //エラーがあった場合
        Logger::Log(shaderError->GetStringPointer());
        assert(false);
    }
    //compile結果をうけとってわたす
    Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
    hr_ = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
    assert(SUCCEEDED(hr_));
    Logger::Log(StringUtility::ConvertString(std::format(L"Compile Succeeded, path:{},profiale:{}\n", filePath, profile)));
    //解放
    return shaderBlob;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DXCommon::CreateBufferResource(size_t sizeInBytes)
{
    //リソース用ヒープ
    D3D12_HEAP_PROPERTIES uploadHeapProperties{};
    uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//アップロードヒープ
    //リソース
    D3D12_RESOURCE_DESC resourceDesc{};
    //バッファリソース
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = sizeInBytes;//リソースのサイズ
    // バッファのサイズ
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    //
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    //リソースを作る
    Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
    hr_ = device_.Get()->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&resource)
    );
    assert(SUCCEEDED(hr_));
    return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DXCommon::CreateTextureResourse(const DirectX::TexMetadata& metadata)
{
    ///metadataを基にリソースを作成
    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Width = UINT(metadata.width);//幅
    resourceDesc.Height = UINT(metadata.height);//高さ
    resourceDesc.MipLevels = UINT16(metadata.mipLevels);//ミップマップの数    
    resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);//配列の数
    resourceDesc.Format = metadata.format;//フォーマット
    resourceDesc.SampleDesc.Count = 1;//サンプル数
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);//リソースの次元
    //利用するheapの設定
    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//デフォルトヒープ
    //リソースの生成
    Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
    HRESULT hr = device_.Get()->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&resource)
    );
    assert(SUCCEEDED(hr));
    return resource;
}
[[nodiscard]]
Microsoft::WRL::ComPtr<ID3D12Resource> DXCommon::UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource> textur, const DirectX::ScratchImage& mipImages)
{

    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    DirectX::PrepareUpload(
        device_.Get(),
        mipImages.GetImages(),
        mipImages.GetImageCount(),
        mipImages.GetMetadata(),
        subresources
    );
    uint64_t intermediateSize = GetRequiredIntermediateSize(
        textur.Get(),
        0,//最初のサブリソース
        UINT(subresources.size())//全てのサブリソース
    );
    Microsoft::WRL::ComPtr< ID3D12Resource> intermediateResource = CreateBufferResource(intermediateSize);
    UpdateSubresources(
        commandList_.Get(),
        textur.Get(),//転送先のテクスチャ
        intermediateResource.Get(),//転送元のリソース
        0,//転送元のオフセット
        0,//転送先のオフセット
        UINT(subresources.size()),//サブリソースの数
        subresources.data()//サブリソースデータ
    );
    //
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;//リソースの遷移
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;//フラグなし
    barrier.Transition.pResource = textur.Get();//遷移するリソース
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;//全てのサブリソース
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;//コピー先の状態
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;//読み取り可能な状態
    commandList_.Get()->ResourceBarrier(1, &barrier);//バリアを設定
    return intermediateResource;

}



void DXCommon::InitializeFixFPS()
{
    reference_ = std::chrono::steady_clock::now();
}

void DXCommon::UpdateFixFPS()
{
    const std::chrono::microseconds kMinTime(uint64_t(1000000.0f/60.0f));
    const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f/65.0f));

    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);
    if (elapsed<kMinCheckTime){
        while (std::chrono::steady_clock::now()-reference_<kMinTime)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(1));

        }

    }
      reference_ = std::chrono::steady_clock::now();
}

void DXCommon::CreateDevice()
{

#ifdef _DEBUG

    //デバッグレイヤーの有効
    Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        //デバッグレイヤーの詳細な情報を取得
        debugController->SetEnableGPUBasedValidation(TRUE);

    }

#endif // _DEBUG
    // 
    dxgiFactory_ = nullptr;

    hr_ = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
    //
    assert(SUCCEEDED(hr_));
    //アダプターの作成
    Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
    //IDXGIAdapter4* useAdapter = nullptr;
    //良い順番のアダプターを探す
    for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
        IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {
        ///アダプターの情報を取得
        DXGI_ADAPTER_DESC3 adapterDesc{};
        hr_ = useAdapter.Get()->GetDesc3(&adapterDesc);
        assert(SUCCEEDED(hr_));
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
            Logger::Log(StringUtility::ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
            break;
        }
        useAdapter = nullptr;
    }
    assert(useAdapter != nullptr);

    device_ = nullptr;

    //対応するFeatureLevelでデバイスを作成
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0,
    };
    //
    const char* featureLevelStrings[] = { "12_2", "12_1", "12_0" };
    for (size_t i = 0; i < _countof(featureLevels); i++) {
        hr_ = D3D12CreateDevice(
            useAdapter.Get(),
            featureLevels[i],
            IID_PPV_ARGS(&device_));

        if (SUCCEEDED(hr_)) {
            Logger::Log((std::format("Use FeatureLevel : {}\n", featureLevelStrings[i])));
            break;

        }
    }
    //
    assert(device_ != nullptr);

#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
    if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue))))
    {
        ///深刻なエラーを出力・停止
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        ///エラーを出力・停止
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        ///警告を出力/停止
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
        //メッセージID
        D3D12_MESSAGE_ID denyIds[] = {
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,

        };
        //
        D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_INFO_QUEUE_FILTER filter{};
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;
        //
        infoQueue->PushStorageFilter(&filter);

    }


#endif // _DEBUG

}

void DXCommon::CreateCommand()
{

    //コマンドキューの作成
    commandQueue_ = nullptr;
    // ID3D12CommandQueue* commandQueue = nullptr;
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    hr_ = device_->CreateCommandQueue(
        &commandQueueDesc,
        IID_PPV_ARGS(&commandQueue_));
    assert(SUCCEEDED(hr_));
    //コマンドアロケーターの作成
    commandAllocator_ = nullptr;
    //ID3D12CommandAllocator* commandAllocator = nullptr;
    hr_ = device_->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&commandAllocator_));
    assert(SUCCEEDED(hr_));
    //コマンドリストの作成
    commandList_ = nullptr;
    //ID3D12GraphicsCommandList* commandList = nullptr;
    hr_ = device_->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandAllocator_.Get(), nullptr,
        IID_PPV_ARGS(&commandList_)
    );
    assert(SUCCEEDED(hr_));
}

void DXCommon::CreateSwapChain()
{

    //スワップチェーンの作成
    swapChain_ = nullptr;
    // IDXGISwapChain4* swapChain = nullptr;

    swapChainDesc_.Width = WinApp::kClientWidth;//画像の幅
    swapChainDesc_.Height = WinApp::kClientHeight;//画像の高さ
    swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
    swapChainDesc_.SampleDesc.Count = 1;//マルチサンプルしない
    swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//レンダリングターゲットとして使用
    swapChainDesc_.BufferCount = 2;//バッファの数
    swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//写したら破棄
    // コマンドキュー,ウィンドウハンドル、設定して生成
    hr_ = dxgiFactory_->CreateSwapChainForHwnd(
        commandQueue_.Get(),
        WinApp::GetInstance()->GetHwnd(),
        &swapChainDesc_,
        nullptr,
        nullptr,
        reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf())
    );
    assert(SUCCEEDED(hr_));
}



void DXCommon::CreateDepthStencilTextureResource() {
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = WinApp::GetInstance()->kClientWidth;//幅
    resourceDesc.Height = WinApp::GetInstance()->kClientHeight;//高さ
    resourceDesc.MipLevels = 1;//ミップマップの数
    resourceDesc.DepthOrArraySize = 1;//配列の数
    resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーマット
    resourceDesc.SampleDesc.Count = 1;//サンプル数
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//リソースの次元
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//深度ステンシルを許可
    //利用するheapの設定
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//デフォルトヒープ
    // 深度値のクリア設定    
    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.DepthStencil.Depth = 1.0f;//深度値のクリア値
    depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーマット
    //リソースの生成
    depthStencilResource_ = nullptr;
    HRESULT hr = device_->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度書き込み状態
        &depthClearValue,//深度値のクリア設定
        IID_PPV_ARGS(&depthStencilResource_)
    );
    assert(SUCCEEDED(hr));
}

void DXCommon::CreateDescriptorHeaps()
{
   // descriptorSizeSRV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    descriptorSizeRTV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    descriptorSizeDSV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    //SRVヒープの作成
   // srvHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);
    //RTVヒープの作成
    rtvHeap_ = CreateDescriptorHeap( D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
    //DSVヒープの作成
    dsvHeap_ = CreateDescriptorHeap( D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);




}

Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> DXCommon::CreateDescriptorHeap( D3D12_DESCRIPTOR_HEAP_TYPE heepType, UINT numDescriptors, bool shaderVisible)
{
    //ディスクリプタヒープの設定
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
    heapDesc.NumDescriptors = numDescriptors;
    heapDesc.Type = heepType;
    heapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
    HRESULT hr = device_.Get()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));
    assert(SUCCEEDED(hr));
    return descriptorHeap;
}

void DXCommon::CreateRenderTargetView()
{
    //スワップチェーンからリソースをひっぱる
    hr_ = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResources_[0]));
    assert(SUCCEEDED(hr_));
    hr_ = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResources_[1]));
    assert(SUCCEEDED(hr_));
    // RTVの作成

    rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換・書き込み
    rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2Dテクスチャ

    //ディスクリプタヒープのハンドルを取得
    for (uint32_t i = 0; i < 2; i++)
    {

        rtvHandles_[i] = GetCPUDescriptorHandle(rtvHeap_, descriptorSizeRTV_, i);
        //レンダーターゲットビューの生成
        device_->CreateRenderTargetView(
            swapChainResources_[i].Get(),
            &rtvDesc_,
            rtvHandles_[i]
        );
    }
}
D3D12_CPU_DESCRIPTOR_HANDLE DXCommon::GetCPUDescriptorHandle(const  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (descriptorSize * index);
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DXCommon::GetGPUDescriptorHandle(const  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (descriptorSize * index);
    return handleGPU;
}

void DXCommon::CreateDepthStencilView()
{
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//深度ステンシルのフォーマット
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2Dテクスチャ
    device_->CreateDepthStencilView(
        depthStencilResource_.Get(),
        &dsvDesc,
        dsvHeap_->GetCPUDescriptorHandleForHeapStart()
    );
}

void DXCommon::CreateFence()
{


    uint64_t fenceValue = 0;
    hr_ = device_->CreateFence(
        fenceValue,
        D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&fence_)
    );
    assert(SUCCEEDED(hr_));
    /* fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
     assert(fenceEvent_ != nullptr);*/
}

void DXCommon::CreateViewport()
{
    viewport_.Width = static_cast<float>(WinApp::kClientWidth);
    viewport_.Height = static_cast<float>(WinApp::kClientHeight);
    viewport_.TopLeftX = 0.0f;
    viewport_.TopLeftY = 0.0f;
    viewport_.MinDepth = 0.0f;
    viewport_.MaxDepth = 1.0f;
}

void DXCommon::CreateScissorRect()
{
    scissorRect_.left = 0;
    scissorRect_.right = WinApp::kClientWidth;
    scissorRect_.top = 0;
    scissorRect_.bottom = WinApp::kClientHeight;
}

void DXCommon::CreateDXCompiler()
{
    hr_ = DxcCreateInstance(
        CLSID_DxcUtils,
        IID_PPV_ARGS(&dxcUtils)
    );
    assert(SUCCEEDED(hr_));
    hr_ = DxcCreateInstance(
        CLSID_DxcCompiler,
        IID_PPV_ARGS(&dxcCompiler)
    );
    assert(SUCCEEDED(hr_));
    hr_ = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
    assert(SUCCEEDED(hr_));

}



