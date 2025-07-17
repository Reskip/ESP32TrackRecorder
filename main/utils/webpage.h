#ifndef WEBPAGE
#define WEBPAGE


const char *html = R"(
<!DOCTYPE html>
<html lang="zh">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no,viewport-fit=cover"/>
    <title>ESP32 Track Recorder</title>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/leaflet@1.9.4/dist/leaflet.css"/>
    <style>
        html, body { height: 100%; margin: 0; padding: 0; overflow: hidden; }
        body { font-family: 'PingFang SC', 'Microsoft YaHei', Arial, sans-serif; }
        #map { height: 100vh; width: 100vw; position: absolute; top: 0; left: 0; z-index: 1; }
        .sidebar {
            position: absolute; top: 0; right: 0; height: 100vh; width: 25%; min-width: 320px; max-width: 480px; background: rgba(255,255,255,0.96);
            box-shadow: -2px 0 10px 0 rgba(0,0,0,0.12); z-index: 30;
            transform: translateX(100%);
            transition: transform 0.25s ease-out;
            flex-direction: column;
            display: flex;
        }
        .sidebar.open {
            transform: translateX(0);
        }
        @media (max-width:600px) {
            .sidebar {
                width: 100vw !important;
                left: 0;
                right: 0;
                top: 0;
                height: 100vh !important;
            }
        }
        .sidebar-header { display: none; }
        .sidebar-section { 
            padding: 18px 18px 0 18px; 
            flex: 1 1 auto; 
            display: flex; 
            flex-direction: column;
            min-height: 0;
            height: 0;
        }
        .polar-plot-wrap {
            display: flex;
            justify-content: center;
            align-items: center;
            width: 100%;
        }
        .polar-plot {
            background: #fff; border-radius: 8px; margin-bottom: 22px; box-shadow: 0 2px 8px #0001;
            width: 240px; height: 240px; flex: 0 0 auto;
            display: flex; justify-content: center; align-items: center;
        }
        .snr-barplot {
            background: #fff; border-radius: 8px; box-shadow: 0 2px 8px #0001; 
            width: 100%; 
            height: 130px;
            margin-bottom: 22px;
            overflow-x: auto;
            overflow-y: hidden;
            flex: 0 0 auto;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .snr-barplot-inner {
            height: 130px;
            min-width: 100%;
            display: block;
            margin: 0 auto;
        }
        .sat-list {
            background: #fff; border-radius: 8px; box-shadow: 0 2px 8px #0001; 
            width: 100%; 
            flex: 1 1 0px;
            min-height: 60px;
            max-height: none;
            overflow-y: auto;
            margin-bottom: 22px;
            padding: 0 0 15px 0;
        }
        .sat-list table { width: 100%; font-size: 13px; border-collapse: collapse; }
        .sat-list th, .sat-list td { padding: 2px 4px; text-align: center; }
        .tile-switcher, .infobar {
            position: absolute;
            left: 18px;
            margin: 10px 0;
        }
        .tile-switcher { top: 18px; z-index: 20; background: #fff; border-radius: 18px; box-shadow: 0 2px 6px #0002; padding: 4px 10px; }
        .tile-switcher button { background: none; border: none; font-size: 15px; color: #333; margin-right: 10px; cursor: pointer; }
        .tile-switcher .active { color: #ff9800; font-weight: bold; }
        .sidebar-toggle-btn, .sdcard-btn, .locate-btn {
            position: absolute; 
            right: 18px;
            background: #ff9800; 
            border-radius: 50%; 
            width: 44px; 
            height: 44px; 
            border: none; 
            color: #fff; 
            font-size: 24px; 
            box-shadow: 0 2px 8px rgba(0,0,0,0.2);
            display: flex; 
            align-items: center; 
            justify-content: center; 
            cursor: pointer;
            transition: all 0.2s ease;
            touch-action: manipulation;
            margin: 10px 0;
        }
        .sidebar-toggle-btn:hover, .sdcard-btn:hover, .locate-btn:hover {
            transform: scale(1.0);
        }
        .sidebar-toggle-btn { top: env(safe-area-inset-top, 28px); z-index: 40; }
        .sdcard-btn { top: calc(env(safe-area-inset-top, 28px) + 52px); z-index: 41; }
        .locate-btn { bottom: env(safe-area-inset-bottom, 34px); z-index: 41; }
        /* SD卡文件列表样式 */
        .sdcard-file-list {
            background: #fff; border-radius: 8px; box-shadow: 0 2px 8px #0001;
            padding: 16px 12px 12px 12px; margin: 16px; max-height: 90vh; min-height: 80px; overflow-y: auto; font-size: 14px;
        }
        .sdcard-file-list table { width: 100%; border-collapse: collapse; }
        .sdcard-file-list th, .sdcard-file-list td { padding: 4px 6px; text-align: left; white-space: nowrap; }
        .infobar {
            top: calc(env(safe-area-inset-top, 28px) + 52px); z-index: 21;
            background: #fff; border-radius: 8px; box-shadow: 0 2px 6px #0002;
            padding: 10px 14px; font-size: 16px; color: #222; min-width: 170px;
            display: flex; flex-direction: column; gap: 4px;
            user-select: none;
        }
        .infobar .status {
            display: inline-block; padding: 1px 10px; border-radius: 8px;
            color: #fff; font-weight: bold; font-size: 14px;
        }
        .infobar .status.recording { background: #44c940; }
        .infobar .status.stopped { background: #888; }
        @media (max-width:500px) {
            .sidebar { width: 100vw !important; }
            .infobar { left: 18px !important; top: calc(env(safe-area-inset-top, 28px) + 52px); min-width: 100px; font-size: 14px; }
            .tile-switcher { left: 18px !important; top: env(safe-area-inset-top, 28px); }
            .sidebar-toggle-btn { top: env(safe-area-inset-top, 28px); }
            .sdcard-btn { top: calc(env(safe-area-inset-top, 28px) + 52px); }
            .locate-btn { bottom: env(safe-area-inset-bottom, 34px); }
        }
        /* 文件操作模态框样式 */
        .modal {
            display: none;
            position: fixed;
            z-index: 100;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            overflow: auto;
            background-color: rgba(0,0,0,0.4);
        }
        .modal-content {
            background-color: #fefefe;
            margin: 15% auto;
            padding: 20px;
            border: 1px solid #888;
            width: 300px;
            border-radius: 8px;
            box-shadow: 0 4px 12px rgba(0,0,0,0.15);
            position: relative;
        }
        .modal-header {
            margin-bottom: 15px;
            font-weight: bold;
            font-size: 16px;
            word-break: break-all;
            max-width: 100%;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
        }
        .modal-actions {
            display: flex;
            justify-content: space-between;
            margin-top: 20px;
            transition: all 0.3s ease;
        }
        .btn-action {
            padding: 8px 16px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 14px;
            transition: all 0.2s;
        }
        .btn-action:hover {
            opacity: 0.9;
        }
        .btn-danger {
            background-color: #f44336;
            color: white;
        }
        .warning-text {
            color: #f44336;
            font-weight: bold;
            margin-bottom: 15px;
        }
        .delete-confirm {
            display: none;
            margin-top: 15px;
        }
        .file-item {
            display: block;
            width: 100%;
            padding: 8px 12px;
            margin: 2px 0;
            background-color: #f0f0f0;
            border: 1px solid #ddd;
            border-radius: 4px;
            color: #333;
            text-align: left;
            text-decoration: none;
            cursor: pointer;
            transition: all 0.2s;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
            vertical-align: middle;
            text-align: center;
        }
        .file-item:hover {
            background-color: #e0e0e0;
            border-color: #bbb;
        }
        .file-item:active {
            background-color: #d0d0d0;
        }
        /* 加载动画 */
        .loading-spinner {
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100%;
        }
        .spinner {
            width: 40px;
            height: 40px;
            border: 4px solid #f3f3f3;
            border-top: 4px solid #ff9800;
            border-radius: 50%;
            animation: spin 1s linear infinite;
        }
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        /* 禁用状态样式 */
        .sidebar-toggle-btn.disabled, .sdcard-btn.disabled {
            opacity: 0.6;
            pointer-events: none;
        }
        /* 新增样式 */
        .file-action-message {
            margin-top: 15px;
            padding: 8px 12px;
            border-radius: 4px;
            text-align: center;
            display: none;
            transition: all 0.3s ease;
        }
        .message-success {
            background-color: #d4edda;
            color: #155724;
        }
        .message-error {
            background-color: #f8d7da;
            color: #721c24;
        }
        .message-loading {
            background-color: #f8f9fa;
            color: #6c757d;
        }
        .return-btn {
            display: none;
            margin-top: 15px;
            width: 100%;
            padding: 8px 16px;
            background-color: #6c757d;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 14px;
            transition: all 0.2s;
        }
        .return-btn:hover {
            background-color: #5a6268;
        }
        .current-file {
            padding: 8px 12px;
            margin: 2px 0;
            background-color: #e8f5e9;
            border-color: #e8f5e9;
            color: #2e7d32;
            font-weight: bold;
            border: 1px solid #e8f5e9;
            border-radius: 4px;
            text-align: center;
            cursor: default !important;
        }
        .current-file:hover {
            background-color: #e8f5e9 !important;
            border-color: #e8f5e9 !important;
        }
    </style>
    <script src="https://cdn.jsdelivr.net/npm/leaflet@1.9.4/dist/leaflet.js"></script>
</head>
<body>
    <div id="map"></div>
    <div class="tile-switcher">
        <button id="btn-gaode" class="active">高德地图</button>
        <button id="btn-osm">OSM</button>
    </div>
    <div class="infobar" id="infobar">
      <span id="trackStatus"></span>
      <span id="trackLength"></span>
      <span id="currentSpeed"></span>
    </div>
    <button class="sidebar-toggle-btn" id="sidebarToggle" title="卫星信息">&#9776;</button>
    <button class="sdcard-btn" id="sdcardBtn" title="SD卡文件">&#128190;</button>
    <button class="locate-btn" id="locateBtn" title="定位">&#128204;</button>
    <div class="sidebar" id="sidebar">
        <div class="sidebar-header"></div>
        <div class="sidebar-section">
            <div class="polar-plot-wrap">
                <div class="polar-plot" id="polarPlot"></div>
            </div>
            <div class="snr-barplot"><div class="snr-barplot-inner" id="snrBar"></div></div>
            <div class="sat-list" id="satList"></div>
        </div>
        <div class="sdcard-file-list" id="sdcardFileList" style="display:none;">
            <div class="loading-spinner">
                <div class="spinner"></div>
            </div>
        </div>
    </div>
    
    <!-- 文件操作模态框 -->
    <div id="fileModal" class="modal">
        <div class="modal-content">
            <div class="modal-header" id="modalFileName">文件操作</div>
            <div class="modal-actions" id="modalActions">
                <button id="downloadBtn" class="btn-action">下载</button>
                <button id="deleteBtn" class="btn-action btn-danger">删除</button>
            </div>
            <div id="deleteConfirm" class="delete-confirm" style="display: none;">
                <p class="warning-text">确定要删除这个文件吗？</p>
                <div class="modal-actions">
                    <button id="confirmDeleteBtn" class="btn-action btn-danger">确认删除</button>
                    <button id="cancelDeleteBtn" class="btn-action">取消</button>
                </div>
            </div>
            <div id="fileActionMessage" class="file-action-message"></div>
            <button id="returnBtn" class="return-btn">返回</button>
        </div>
    </div>
    
<script>
// 卫星类型与配色
function satTypeInfo(type) {
    if (!type) return { label: "其它", color: "#bdbdbd" };
    if (type.startsWith("GP")) return { label: "GPS", color: "#ff9800" };        // 橙
    if (type.startsWith("GL")) return { label: "格洛纳斯", color: "#2196f3" };  // 蓝
    if (type.startsWith("GB")) return { label: "北斗", color: "#43a047" };      // 绿
    return { label: "其它", color: "#bdbdbd" };                                 // 灰
}

// 卫星系统排序权重
function getSatSystemWeight(type) {
    if (type.startsWith("GP")) return 1; // GPS
    if (type.startsWith("GL")) return 2; // 格洛纳斯
    if (type.startsWith("GB")) return 3; // 北斗
    return 4; // 其它
}

// 排序卫星数据：先按系统类型，再按SNR
function sortSatellites(sats) {
    return [...sats].sort((a, b) => {
        const systemA = getSatSystemWeight(a.type);
        const systemB = getSatSystemWeight(b.type);
        
        if (systemA !== systemB) {
            return systemA - systemB; // 系统类型排序
        }
        
        // 相同系统内按SNR降序排列
        const snrA = a.snr || 0;
        const snrB = b.snr || 0;
        return snrB - snrA;
    });
}

// 地图与轨迹相关
let map, tileGaode, tileOSM, trackLayer, currentTrack = null;
const gcj02TileUrl = 'https://webrd0{s}.is.autonavi.com/appmaptile?lang=zh_cn&size=1&scale=1&style=8&x={x}&y={y}&z={z}';

// WGS84 to GCJ-02工具
const transform = {
    PI: 3.1415926535897932384626, A: 6378245.0, EE: 0.00669342162296594323,
    wgs84ToGcj02(lng, lat) {
        if (this.outOfChina(lng, lat)) return [lng, lat];
        let dlat = this.transformLat(lng - 105.0, lat - 35.0);
        let dlng = this.transformLon(lng - 105.0, lat - 35.0);
        let radlat = lat / 180.0 * this.PI;
        let magic = Math.sin(radlat);
        magic = 1 - this.EE * magic * magic;
        let sqrtmagic = Math.sqrt(magic);
        dlat = (dlat * 180.0) / ((this.A * (1 - this.EE)) / (magic * sqrtmagic) * this.PI);
        dlng = (dlng * 180.0) / (this.A / sqrtmagic * Math.cos(radlat) * this.PI);
        let mglat = lat + dlat, mglng = lng + dlng;
        return [mglng, mglat];
    },
    outOfChina(lng, lat) {
        return (lng < 72.004 || lng > 137.8347 || lat < 0.8293 || lat > 55.8271);
    },
    transformLat(x, y) { let ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * Math.sqrt(Math.abs(x)); ret += (20.0 * Math.sin(6.0 * x * this.PI) + 20.0 * Math.sin(2.0 * x * this.PI)) * 2.0 / 3.0; ret += (20.0 * Math.sin(y * this.PI) + 40.0 * Math.sin(y / 3.0 * this.PI)) * 2.0 / 3.0; ret += (160.0 * Math.sin(y / 12.0 * this.PI) + 320 * Math.sin(y * this.PI / 30.0)) * 2.0 / 3.0; return ret; },
    transformLon(x, y) { let ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * Math.sqrt(Math.abs(x)); ret += (20.0 * Math.sin(6.0 * x * this.PI) + 20.0 * Math.sin(2.0 * x * this.PI)) * 2.0 / 3.0; ret += (20.0 * Math.sin(x * this.PI) + 40.0 * Math.sin(x / 3.0 * this.PI)) * 2.0 / 3.0; ret += (150.0 * Math.sin(x / 12.0 * this.PI) + 300.0 * Math.sin(x / 30.0 * this.PI)) * 2.0 / 3.0; return ret; }
};

// ========== 重新定位相关 ==========
let firstLocate = true;
let lastUserAction = Date.now();
let lastLocatedPoint = null;
let lastPositionCoords = null;

let lastTraceData = [];
let lastCourse = null;
let positionCircle = null;
let positionTriangle = null;
let lastTraceReqTime = Date.now();

// 监听用户地图操作（拖动/缩放等行为）
function setupMapActionListener() {
    if (!map) return;
    ['movestart', 'zoomstart', 'dragstart'].forEach(ev => {
        map.on(ev, function() {
            lastUserAction = Date.now();
        });
    });

    map.on('zoomend', function() {
        updatePositionMarker(lastTraceData, lastCourse);
        console.log("reset");
    });
}

function handleLatestLocation(traceArr) {
    if (!traceArr || traceArr.length === 0) return;
    let latest = traceArr[traceArr.length - 1];
    let latestCoord = latest.lat + ',' + latest.lon;

    // 检查是否有新的定位点
    const hasNewPosition = lastPositionCoords !== latestCoord;
    lastPositionCoords = latestCoord;

    // 第一次定位自动居中
    if (firstLocate) {
        let [lon, lat] = transform.wgs84ToGcj02(latest.lon, latest.lat)
        map.setView([lat, lon], map.getZoom() || 16);
        lastLocatedPoint = latestCoord;
        firstLocate = false;
        return;
    }

    // 10秒无操作且有新定位点时才自动定位
    if ((Date.now() - lastUserAction) > 10000 && hasNewPosition) {
        let _lastUserAction = lastUserAction;
        let [lon, lat] = transform.wgs84ToGcj02(latest.lon, latest.lat)
        map.panTo([lat, lon]);
        lastLocatedPoint = latestCoord;
        lastUserAction = _lastUserAction;
    }
}

function updatePositionMarker(traceArr, course) {
    if (!traceArr || traceArr.length === 0) return;
    
    const latestPoint = traceArr[traceArr.length - 1];
    const [lon, lat] = transform.wgs84ToGcj02(latestPoint.lon, latestPoint.lat);
    const point = L.latLng(lat, lon);

    if (!positionCircle) {
        positionCircle = L.circleMarker(point, {
            radius: 10,
            fillColor: "#2196f3",
            color: "transparent",
            weight: 0,
            opacity: 1,
            fillOpacity: 0.6
        }).addTo(map);
    } else {
        positionCircle.setLatLng(point);
    }

    if (course !== null) {
        if (!positionTriangle) {
            positionTriangle = L.polygon([], {
                fillColor: "#2196f3",
                weight: 1.5,
                opacity: 0.3,
                fillOpacity: 0.6,
                lineCap: 'round',
                lineJoin: 'round',
                color: '#2196f3'
            }).addTo(map);
        }
        const ang = (course || 0) * Math.PI / 180;
        const half = Math.PI / 6;

        const center = map.latLngToLayerPoint(point);

        function offset(angle, dist) {
            return L.point(
                center.x + dist * Math.sin(angle),
                center.y - dist * Math.cos(angle)
            );
        }

        const apex = offset(ang, 16);
        const apex_close = offset(ang, 14);
        const left = offset(ang + half, 14);
        const right = offset(ang - half, 14);

        positionTriangle.setLatLngs([
            map.layerPointToLatLng(apex),
            map.layerPointToLatLng(left),
            map.layerPointToLatLng(apex_close),
            map.layerPointToLatLng(right)
        ]);
        if (!map.hasLayer(positionTriangle)) {
            positionTriangle.addTo(map);
        }
    } else {
        if (positionTriangle && map.hasLayer(positionTriangle)) {
            map.removeLayer(positionTriangle);
        }
    }
}

function initMap() {
    map = L.map('map', { zoomControl: false, attributionControl: false }).setView([39.9042, 116.4074], 13);
    tileGaode = L.tileLayer(gcj02TileUrl, { subdomains: '1234', maxZoom: 18, minZoom: 3, attribution: '© 高德' });
    tileOSM = L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', { maxZoom: 19, attribution: '© OSM' });
    tileGaode.addTo(map);
    trackLayer = L.layerGroup().addTo(map);
    setupMapActionListener();
    
    // 监听窗口大小变化，调整地图尺寸
    window.addEventListener('resize', () => {
        if (map) {
            setTimeout(() => {
                map.invalidateSize();
            }, 100);
        }
    });
    
    // 处理页面可见性变化
    document.addEventListener('visibilitychange', () => {
        if (document.visibilityState === 'visible') {
            // 页面从后台切换回来时，重新调整地图尺寸
            setTimeout(() => {
                map.invalidateSize();
            }, 300);
        }
    });
}
initMap();

// 切换底图
function switchTile(useGaode) {
    if (useGaode) {
        map.removeLayer(tileOSM); tileGaode.addTo(map);
        document.getElementById('btn-gaode').classList.add('active');
        document.getElementById('btn-osm').classList.remove('active');
    } else {
        map.removeLayer(tileGaode); tileOSM.addTo(map);
        document.getElementById('btn-gaode').classList.remove('active');
        document.getElementById('btn-osm').classList.add('active');
    }
    drawTrack(lastTraceData);
}
document.getElementById('btn-gaode').onclick = () => switchTile(true);
document.getElementById('btn-osm').onclick = () => switchTile(false);

// 侧边栏状态管理
let sidebarOpen = false;
let currentSidebarContent = null; // 'satellite' 或 'sdcard'
let isTransitioning = false; // 过渡状态标志
let isLoadingFiles = false; // 文件加载状态标志

// 侧边栏动画+display控制
const sidebar = document.getElementById('sidebar');
const toggleBtn = document.getElementById('sidebarToggle');
const sdcardBtn = document.getElementById('sdcardBtn');
const sidebarSection = document.querySelector('.sidebar-section');
const sdcardFileList = document.getElementById('sdcardFileList');

// 等待侧边栏动画完成的Promise
function waitForTransition(element) {
    return new Promise(resolve => {
        const handler = () => {
            element.removeEventListener('transitionend', handler);
            resolve();
        };
        element.addEventListener('transitionend', handler);
        // 设置超时，防止动画未触发
        setTimeout(resolve, 300);
    });
}

function updateSdcardPage() {
    sdcardFileList.innerHTML = `
        <div class="loading-spinner">
            <div class="spinner"></div>
        </div>
    `;

    setTimeout(async () => {
        isLoadingFiles = true;
        try {
            const response = await fetch('/sdcard_files');
            if (!response.ok) throw new Error('Failed to fetch files');
            
            const data = await response.json();
            let files = data.files || [];
            let currentFile = data.current_file || '';
            files.sort((a, b) => {
                return b.name.localeCompare(a.name);
            });
            let html = `<div style="font-weight:bold; font-size:16px; margin-bottom:10px;">SD卡文件</div>`;
            
            if (!files.length) {
                html += `<div style="color:#888;">未发现文件</div>`;
            } else {
                html += `<table><tr><th>文件名</th><th style="width:72px;">大小</th></tr>`;
                for (let f of files) {
                    let size = f.size;
                    if (size > 1e6) size = (size/1e6).toFixed(1) + ' MB';
                    else if (size > 1e3) size = (size/1e3).toFixed(1) + ' KB';
                    else size = size + ' B';
                    const isCurrentFile = currentFile.includes(f.name);
                    if (isCurrentFile) {
                        html += `<tr>
                            <td>
                                <div class="current-file" title="${f.name}">
                                    ${f.name}
                                </div>
                            </td>
                            <td>${size}</td>
                        </tr>`;
                    } else {
                        html += `<tr>
                            <td>
                                <button type="button" class="file-item" data-file="${f.name}" title="${f.name}">
                                    ${f.name}
                                </button>
                            </td>
                            <td>${size}</td>
                        </tr>`;
                    }
                }
                html += `</table>`;
            }

            sdcardFileList.innerHTML = html;

            setTimeout(() => {
                document.querySelectorAll('.file-item').forEach(item => {
                    item.addEventListener('click', function() {
                        const fileName = this.getAttribute('data-file');
                        document.getElementById('modalFileName').textContent = fileName;
                        resetFileModal();
                        document.getElementById('fileModal').style.display = 'block';
                    });
                });
            }, 100);
        } catch (error) {
            console.error('Error loading SD card files:', error);
            sdcardFileList.innerHTML = `
                <div style="color:red; text-align:center; padding:20px;">
                    加载文件失败，请重试
                </div>
            `;
        } finally {
            isLoadingFiles = false;
            isTransitioning = false;
        }
    }, 200);
}

async function openSidebar(contentType) {
    // 如果正在过渡或正在加载文件，不执行任何操作
    if (isTransitioning || (contentType === 'sdcard' && isLoadingFiles)) return;

    isTransitioning = true;

    if (currentSidebarContent === contentType) {
        // 关闭当前侧边栏
        sidebar.classList.remove('open');
        await waitForTransition(sidebar);
        sidebarSection.style.display = '';
        sdcardFileList.style.display = 'none';
        currentSidebarContent = null;
        sidebarOpen = false;
        isTransitioning = false;
        return;
    }
    
    // 如果打开了不同的内容，先关闭当前内容
    if (sidebarOpen) {
        sidebar.classList.remove('open');
        await waitForTransition(sidebar);
    }

    // 切换内容
    sidebarSection.style.display = contentType === 'satellite' ? '' : 'none';
    sdcardFileList.style.display = contentType === 'sdcard' ? '' : 'none';

    if (contentType === 'sdcard') {
        updateSdcardPage();
    } else {
        isTransitioning = false;
    }
    
    sidebar.classList.add('open');
    sidebarOpen = true;
    currentSidebarContent = contentType;
}

toggleBtn.onclick = () => openSidebar('satellite');
sdcardBtn.onclick = () => openSidebar('sdcard');

sidebar.addEventListener('transitionend', function() {
    if (!sidebar.classList.contains('open') && !isTransitioning) {
        sdcardFileList.style.display = 'none';
        sidebarSection.style.display = '';
        currentSidebarContent = null;
        sidebarOpen = false;
    }
});

document.getElementById('locateBtn').onclick = function() {
    if (lastTraceData && lastTraceData.length) {
        let latest = lastTraceData[lastTraceData.length - 1];
        let [lon, lat] = transform.wgs84ToGcj02(latest.lon, latest.lat)
        map.setView([lat, lon], 18, {animate: true});
        handleLatestLocation(lastTraceData);
    }
};

function drawTrack(traceArr) {
    if (!traceArr || traceArr.length === 0) { 
        if (currentTrack) { 
            trackLayer.removeLayer(currentTrack); 
            currentTrack = null; 
        } 
        return; 
    }
    
    let useGaode = map.hasLayer(tileGaode);
    let points = traceArr.map(pt => {
        if (useGaode) {
            let [lon, lat] = transform.wgs84ToGcj02(pt.lon, pt.lat);
            return [lat, lon];
        } else {
            return [pt.lat, pt.lon];
        }
    });
    
    if (currentTrack) trackLayer.removeLayer(currentTrack);
    currentTrack = L.polyline(points, { color: '#2196f3', weight: 4, opacity: 0.8 }).addTo(trackLayer);
}

function isSamePoint(p1, p2) {
    return p1 && p2 && 
           p1.lat === p2.lat && 
           p1.lon === p2.lon;
}

async function fetchFullTrace() {
    try {
        const response = await fetch('/trace_full');
        if (!response.ok) throw new Error('Failed to fetch full trace');
        
        const data = await response.json();
        let traceArr = data.trace || data;
        lastTraceData = [...traceArr];
        lastCourse = data.course;
        drawTrack(lastTraceData);
        handleLatestLocation(lastTraceData);
        updatePositionMarker(lastTraceData, data.course);

        updateTrackInfo(data);
    } catch (error) {
        console.error('Error fetching full trace:', error);
    }
}

async function fetchRecentTrace() {
    try {
        const response = await fetch('/trace_recent');
        if (!response.ok) throw new Error('Failed to fetch recent trace');
        
        const data = await response.json();
        let newPoints = data.trace || [];

        if (newPoints.length > 0 && !isSamePoint(newPoints[0], lastTraceData[lastTraceData.length - 1])) {
            lastTraceData = [...lastTraceData, ...newPoints];
            drawTrack(lastTraceData);
            handleLatestLocation(lastTraceData);
            updatePositionMarker(lastTraceData, data.course);
        }

        updateTrackInfo(data);
    } catch (error) {
        console.error('Error fetching recent trace:', error);
    }
}

// 更新轨迹信息
function updateTrackInfo(data) {
    let status = data.in_track ? "记录中" : "未在记录";
    let statusClass = data.in_track ? "recording" : "stopped";
    let distance = data.distance || 0;
    let lengthTxt = `轨迹长度: ${(distance).toFixed(2)} km`;
    let speed = data.speed || 0;
    let speedTxt = `当前速度: ${speed.toFixed(1)} km/h`;
    
    document.getElementById('trackStatus').textContent = status;
    document.getElementById('trackStatus').className = `status ${statusClass}`;
    document.getElementById('trackLength').textContent = lengthTxt;
    document.getElementById('currentSpeed').textContent = speedTxt;
}

fetchFullTrace().then(() => { lastTraceReqTime = Date.now(); });

async function autoFetchTrace() {
    let now = Date.now();
    if (now - lastTraceReqTime > 5000) {
        await fetchFullTrace();
    } else {
        await fetchRecentTrace();
    }
    lastTraceReqTime = Date.now();
}

setInterval(autoFetchTrace, 1100);

// ------ 卫星信息相关 ------
async function fetchSatellites() {
    try {
        const response = await fetch('/satellites');
        if (!response.ok) throw new Error('Failed to fetch satellites');
        
        const data = await response.json();
        let sats = data.satellites || [];
        
        // 排序卫星数据
        const sortedSats = sortSatellites(sats);
        
        renderPolarPlot(sortedSats);
        renderSnrBar(sortedSats);
        renderSatList(sortedSats);
    } catch (error) {
        console.error('Error fetching satellites:', error);
    }
}

setInterval(fetchSatellites, 3000);
fetchSatellites();

// 极坐标图
function renderPolarPlot(sats) {
    let c = document.createElement('canvas');
    c.width = c.height = 240;
    let ctx = c.getContext('2d');
    ctx.clearRect(0,0,240,240);
    ctx.save();
    ctx.translate(120,120);
    ctx.strokeStyle = '#bbb';
    ctx.lineWidth = 1.2;
    ctx.beginPath(); ctx.arc(0,0,100,0,2*Math.PI); ctx.stroke();
    ctx.beginPath(); ctx.arc(0,0,60,0,2*Math.PI); ctx.stroke();
    ctx.beginPath(); ctx.arc(0,0,20,0,2*Math.PI); ctx.stroke();
    ctx.strokeStyle = '#ddd';
    for (let i = 0; i < 360; i += 30) {
        let rad = i * Math.PI / 180;
        ctx.beginPath();
        ctx.moveTo(0, 0);
        ctx.lineTo(100 * Math.sin(rad), -100 * Math.cos(rad));
        ctx.stroke();
    }
    for (let sat of sats) {
        if ((sat.snr||0) === 0) continue;
        let az = sat.azimuth || 0, el = sat.elevation || 0;
        let r = 100 * (1 - el / 90);
        let rad = az * Math.PI / 180;
        let info = satTypeInfo(sat.type);
        let cx = r * Math.sin(rad), cy = -r * Math.cos(rad);
        ctx.beginPath();
        ctx.arc(cx, cy, 9, 0, 2 * Math.PI);
        ctx.fillStyle = info.color;
        ctx.globalAlpha = sat.in_use ? 0.55 : 0.19;
        ctx.fill();
        ctx.globalAlpha = 1;
    }
    ctx.restore();
    document.getElementById('polarPlot').innerHTML = '';
    document.getElementById('polarPlot').appendChild(c);
}

// SNR直方图
function renderSnrBar(sats) {
    let minBarW = 12, maxBarW = 24;
    let n = sats.length;
    let width = Math.max(240, n * minBarW + 30);
    let c = document.createElement('canvas');
    c.width = width; c.height = 130;
    let ctx = c.getContext('2d');
    ctx.clearRect(0,0,width,130);

    let bars = sats.map(s=>s.snr||0);
    let barW = Math.max(minBarW, Math.min(maxBarW, Math.floor((width-30)/n)));
    for (let i=0; i<bars.length; ++i) {
        let h = Math.max(2, bars[i]/60*96);
        let info = satTypeInfo(sats[i].type);
        ctx.globalAlpha = sats[i].in_use ? 0.55 : 0.19;
        ctx.fillStyle = info.color;
        ctx.fillRect(18+i*barW, 110-h, barW-3, h);
        ctx.globalAlpha = 1;
    }
    ctx.strokeStyle='#888'; ctx.beginPath(); ctx.moveTo(15,10); ctx.lineTo(15,110); ctx.stroke();
    ctx.font='10px sans-serif'; ctx.fillStyle='#555';
    ctx.textAlign = "right";
    ctx.fillText('60', 13, 16); ctx.fillText('0', 13, 112);

    let barContainer = document.getElementById('snrBar');
    barContainer.innerHTML = '';
    barContainer.appendChild(c);
    barContainer.style.width = "100%";
    barContainer.style.height = "130px";
    c.style.display = "block";
}
 
function renderSatList(sats) {
    let html = `<table><tr><th>型号</th><th>编号</th><th>仰角</th><th>方位</th><th>信噪比</th><th>使用中</th></tr>`;
    for (let s of sats) {
        let info = satTypeInfo(s.type);
        html += `<tr>
        <td style="color:${info.color}; font-weight:bold;">${info.label}</td>
        <td>${s.nr||''}</td>
        <td>${s.elevation||''}</td>
        <td>${s.azimuth||''}</td>
        <td>${s.snr||''}</td>
        <td>${s.in_use?'✔️':''}</td>
        </tr>`;
    }
    let emptyRow = `<tr style="height:24px;"><td>&nbsp;</td><td></td><td></td><td></td><td></td><td></td></tr>`;
    for(let i=0;i<6;++i) html += emptyRow;
    html += `</table>`;
    document.getElementById('satList').innerHTML = html;
}

let currentFileName = '';

function resetFileModal() {
    document.getElementById('modalActions').style.display = 'flex';
    document.getElementById('deleteConfirm').style.display = 'none';
    document.getElementById('fileActionMessage').style.display = 'none';
    document.getElementById('returnBtn').style.display = 'none';
}

function showFileActionMessage(message, type) {
    const messageElement = document.getElementById('fileActionMessage');
    const actionsElement = document.getElementById('modalActions');
    const deleteConfirmElement = document.getElementById('deleteConfirm');
    
    messageElement.textContent = message;
    messageElement.className = 'file-action-message';
    
    if (type === 'success') {
        messageElement.classList.add('message-success');
    } else if (type === 'error') {
        messageElement.classList.add('message-error');
    } else if (type === 'loading') {
        messageElement.classList.add('message-loading');
    }
    
    // 隐藏按钮，显示状态信息
    actionsElement.style.display = 'none';
    deleteConfirmElement.style.display = 'none';
    messageElement.style.display = 'block';
    document.getElementById('returnBtn').style.display = 'block';
}

function downloadFile() {
    currentFileName = document.getElementById('modalFileName').textContent;
    showFileActionMessage('准备下载文件...', 'loading');
    
    setTimeout(() => {
        window.location.href = `/download?file=${encodeURIComponent(currentFileName)}`;
        showFileActionMessage('已开始下载文件', 'success');
    }, 500);
}

async function deleteFile() {
    currentFileName = document.getElementById('modalFileName').textContent;
    showFileActionMessage('正在删除文件...', 'loading');
    
    try {
        const response = await fetch(`/delete?file=${encodeURIComponent(currentFileName)}`);
        
        if (!response.ok) {
            throw new Error('删除文件失败');
        }
        
        await response.json();
        showFileActionMessage('文件已成功删除', 'success');
        
        // 刷新文件列表
        setTimeout(() => {
            updateSdcardPage();
        }, 1000);
    } catch (error) {
        showFileActionMessage(error.message, 'error');
    }
}

// 初始化文件操作模态框
document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('downloadBtn').addEventListener('click', function() {
        downloadFile();
    });
    
    document.getElementById('deleteBtn').addEventListener('click', function() {
        document.getElementById('deleteConfirm').style.display = 'block';
    });

    document.getElementById('confirmDeleteBtn').addEventListener('click', function() {
        deleteFile();
    });

    document.getElementById('cancelDeleteBtn').addEventListener('click', function() {
        document.getElementById('deleteConfirm').style.display = 'none';
    });

    document.getElementById('returnBtn').addEventListener('click', function() {
        document.getElementById('fileModal').style.display = 'none';
        resetFileModal();
    });

    document.getElementById('fileModal').addEventListener('click', function(event) {
        if (event.target === this) {
            resetFileModal();
            this.style.display = 'none';
        }
    });
});
</script>
</body>
</html>
)";

#endif