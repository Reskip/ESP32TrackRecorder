#ifndef WEBPAGE
#define WEBPAGE


const char *html = R"(
<!DOCTYPE html>
<html lang="zh">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalalable=no"/>
    <title>ESP32 Track Recorder</title>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/leaflet@1.9.4/dist/leaflet.css"/>
    <style>
        html, body { height: 100%; margin: 0; padding: 0; overflow: hidden; }
        body { font-family: 'PingFang SC', 'Microsoft YaHei', Arial, sans-serif; }
        #map { height: 100vh; width: 100vw; position: absolute; top: 0; left: 0; z-index: 1; }
        .sidebar {
            position: absolute; top: 0; right: 0; height: 100vh; width: 320px; background: rgba(255,255,255,0.96);
            box-shadow: -2px 0 10px 0 rgba(0,0,0,0.12); z-index: 30;
            display: none;
            transform: translateX(100%);
            transition: transform 0.25s linear;
            flex-direction: column;
        }
        .sidebar.open {
            display: flex;
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
        }
        .sidebar-toggle-btn:hover, .sdcard-btn:hover, .locate-btn:hover {
            transform: scale(1.08);
        }
        .sidebar-toggle-btn { top: 18px; z-index: 40; }
        .sdcard-btn { top: 70px; z-index: 41; }
        .locate-btn { bottom: 24px; z-index: 41; }
        /* SD卡文件列表样式 */
        .sdcard-file-list {
            background: #fff; border-radius: 8px; box-shadow: 0 2px 8px #0001;
            padding: 16px 12px 12px 12px; margin: 16px; max-height: 60vh; min-height: 80px; overflow-y: auto; font-size: 14px;
        }
        .sdcard-file-list table { width: 100%; border-collapse: collapse; }
        .sdcard-file-list th, .sdcard-file-list td { padding: 4px 6px; text-align: left; white-space: nowrap; }
        .sdcard-file-list .fn { max-width: 160px; overflow: hidden; text-overflow: ellipsis; display: inline-block; vertical-align: bottom;}
        .infobar {
            top: 70px; z-index: 21;
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
            .infobar { left: 18px !important; top: 64px; min-width: 100px; font-size: 14px; }
            .tile-switcher { left: 18px !important; }
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
<script>
// 卫星类型与配色
function satTypeInfo(type) {
    if (!type) return { label: "其它", color: "#bdbdbd" };
    if (type.startsWith("GP")) return { label: "GPS", color: "#ff9800" };        // 橙
    if (type.startsWith("GL")) return { label: "格洛纳斯", color: "#2196f3" };  // 蓝
    if (type.startsWith("GB")) return { label: "北斗", color: "#43a047" };      // 绿
    return { label: "其它", color: "#bdbdbd" };                                 // 灰
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

// 监听用户地图操作（拖动/缩放等行为）
function setupMapActionListener() {
    if (!map) return;
    ['movestart', 'zoomstart', 'dragstart'].forEach(ev => {
        map.on(ev, function() {
            lastUserAction = Date.now();
        });
    });
}
function handleLatestLocation(traceArr) {
    if (!traceArr || traceArr.length === 0) return;
    let latest = traceArr[traceArr.length - 1];
    let latestCoord = latest.lat + ',' + latest.lon;

    // 第一次定位自动居中
    if (firstLocate) {
        map.setView([latest.lat, latest.lon], map.getZoom() || 16);
        lastLocatedPoint = latestCoord;
        firstLocate = false;
        return;
    }
    // 10秒无操作且有新定位
    if (Date.now() - lastUserAction > 10000 && lastLocatedPoint !== latestCoord) {
        map.panTo([latest.lat, latest.lon]);
        lastLocatedPoint = latestCoord;
    }
}

// 地图与轨迹相关
let lastTraceData = [];
function initMap() {
    map = L.map('map', { zoomControl: false, attributionControl: false }).setView([39.9042, 116.4074], 13);
    tileGaode = L.tileLayer(gcj02TileUrl, { subdomains: '1234', maxZoom: 18, minZoom: 3, attribution: '© 高德' });
    tileOSM = L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', { maxZoom: 19, attribution: '© OSM' });
    tileGaode.addTo(map);
    trackLayer = L.layerGroup().addTo(map);
    setupMapActionListener();
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
let isTransitioning = false; // 新增：过渡状态标志

// 侧边栏动画+display控制
const sidebar = document.getElementById('sidebar');
const toggleBtn = document.getElementById('sidebarToggle');
const sdcardBtn = document.getElementById('sdcardBtn');
const sidebarSection = document.querySelector('.sidebar-section');
const sdcardFileList = document.getElementById('sdcardFileList');

// 禁用/启用侧边栏按钮
function disableSidebarButtons() {
    isTransitioning = true;
}

function enableSidebarButtons() {
    isTransitioning = false;
}

function openSidebar(contentType) {
    // 如果正在过渡，不执行任何操作
    if (isTransitioning) return;
    
    disableSidebarButtons();
    
    if (currentSidebarContent === contentType) {
        // 关闭当前侧边栏
        sidebar.classList.remove('open');
        setTimeout(() => {
            sidebar.style.display = 'none';
            currentSidebarContent = null;
            sidebarOpen = false;
            enableSidebarButtons();
        }, 250);
        return;
    }
    
    // 如果打开了不同的内容，先关闭当前内容
    if (sidebarOpen) {
        sidebar.classList.remove('open');
        setTimeout(() => {
            // 切换内容
            sidebar.classList.add('open');
            if (contentType === 'satellite') {
                sidebarSection.style.display = '';
                sdcardFileList.style.display = 'none';
            } else if (contentType === 'sdcard') {
                sidebarSection.style.display = 'none';
                sdcardFileList.style.display = '';
                // 显示加载动画
                sdcardFileList.innerHTML = `
                    <div class="loading-spinner">
                        <div class="spinner"></div>
                    </div>
                `;
            }
            
            // 打开侧边栏
            sidebar.style.display = 'flex';
            setTimeout(() => {
                currentSidebarContent = contentType;
                sidebarOpen = true;
                enableSidebarButtons();
            }, 250);
            
        }, 200);
    } else {
        // 直接打开侧边栏
        sidebar.style.display = 'flex';
        setTimeout(() => {
            sidebar.classList.add('open');
            
            if (contentType === 'satellite') {
                sidebarSection.style.display = '';
                sdcardFileList.style.display = 'none';
            } else if (contentType === 'sdcard') {
                sidebarSection.style.display = 'none';
                sdcardFileList.style.display = '';
                // 显示加载动画
                sdcardFileList.innerHTML = `
                    <div class="loading-spinner">
                        <div class="spinner"></div>
                    </div>
                `;
            }
            
            currentSidebarContent = contentType;
            sidebarOpen = true;
            enableSidebarButtons();
        }, 250);
    }
}

toggleBtn.onclick = () => openSidebar('satellite');
sdcardBtn.onclick = () => {
    openSidebar('sdcard');
    // 拉取SD卡文件
    fetch('/sdcard_files').then(r => r.json()).then(data => {
        let files = data.files || [];
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
                html += `<tr><td class="fn" title="${f.name}">${f.name}</td><td>${size}</td></tr>`;
            }
            html += `</table>`;
        }
        sdcardFileList.innerHTML = html;
    }).catch(() => {
    })
};

// 当侧边栏关闭时，恢复内容
sidebar.addEventListener('transitionend', function() {
    if (!sidebar.classList.contains('open')) {
        sdcardFileList.style.display = 'none';
        sidebarSection.style.display = '';
        currentSidebarContent = null;
        sidebarOpen = false;
        enableSidebarButtons();
    }
});

// ========== 右下角定位按钮功能 ==========
document.getElementById('locateBtn').onclick = function() {
    if (lastTraceData && lastTraceData.length) {
        let latest = lastTraceData[lastTraceData.length - 1];
        map.setView([latest.lat, latest.lon], 18, {animate: true});
        handleLatestLocation(lastTraceData);
    }
};

// 轨迹显示
function drawTrack(traceArr) {
    if (!traceArr || traceArr.length === 0) { if (currentTrack) { trackLayer.removeLayer(currentTrack); currentTrack = null; } return; }
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
    currentTrack = L.polyline(points, { color: '#ff9800', weight: 4, opacity: 0.8 }).addTo(trackLayer);
    // 不自动fitBounds，居中逻辑在 handleLatestLocation 控制
}

// 定时获取轨迹
function fetchTrace() {
    fetch('/trace').then(r => r.json()).then(data => {
        let traceArr = data.trace || data;
        lastTraceData = traceArr;
        drawTrack(traceArr);
        handleLatestLocation(traceArr); // 定位逻辑

        // 更新轨迹状态与长度
        let status = data.in_track ? "记录中" : "未在记录";
        let statusClass = data.in_track ? "recording" : "stopped";
        let distance = data.distance || 0;
        let lengthTxt = `轨迹长度: ${(distance).toFixed(2)} km`;
        document.getElementById('trackStatus').textContent = status;
        document.getElementById('trackStatus').className = `status ${statusClass}`;
        document.getElementById('trackLength').textContent = lengthTxt;
    }).catch(()=>{});
}
setInterval(fetchTrace, 5000);
fetchTrace();

// ------ 卫星信息相关 ------
function fetchSatellites() {
    fetch('/satellites').then(r => r.json()).then(data => {
        let sats = data.satellites || [];
        renderPolarPlot(sats);
        renderSnrBar(sats);
        renderSatList(sats);
    }).catch(()=>{});
}
setInterval(fetchSatellites, 2000);
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
</script>
</body>
</html>
)";

#endif