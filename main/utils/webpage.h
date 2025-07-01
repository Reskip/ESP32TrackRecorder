#ifndef WEBPAGE
#define WEBPAGE


const char *html = R"(<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Tracker - 高德地图版</title>
    <!-- 引入Leaflet CSS和JS -->
    <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" integrity="sha256-p4NxAoJBhIIN+hmNHrzRCf9tD/miZyoHS5obTRR9BMY=" crossorigin=""/>
    <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js" integrity="sha256-20nQCchB9co0qIjJZRGuk2/Z9VM+kNiyxNV1lvTlZBo=" crossorigin=""></script>
    <style>
        body, html {
            margin: 0;
            padding: 0;
            height: 100%;
            font-family: "Microsoft YaHei", Arial, sans-serif;
        }
        #map {
            width: 100%;
            height: calc(100vh - 80px);
        }
        .header {
            padding: 10px 20px;
            background-color: #f0f0f0;
            border-bottom: 1px solid #ddd;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .info-panel {
            background-color: white;
            padding: 10px;
            border-radius: 5px;
            box-shadow: 0 1px 5px rgba(0,0,0,0.4);
            margin: 10px;
        }
        .btn {
            background-color: #4CAF50;
            border: none;
            color: white;
            padding: 8px 16px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 14px;
            margin: 4px 2px;
            cursor: pointer;
            border-radius: 4px;
        }
        .btn-refresh {
            background-color: #008CBA;
        }
        .status {
            margin: 0 10px;
            font-weight: bold;
        }
        .status.connected {
            color: green;
        }
        .status.disconnected {
            color: red;
        }
        .tile-note {
            position: absolute;
            bottom: 10px;
            left: 10px;
            background-color: rgba(255,255,255,0.8);
            padding: 5px 10px;
            border-radius: 4px;
            font-size: 12px;
            z-index: 1000;
        }
        .coord-note {
            position: absolute;
            bottom: 35px;
            left: 10px;
            background-color: rgba(255,255,255,0.8);
            padding: 5px 10px;
            border-radius: 4px;
            font-size: 12px;
            z-index: 1000;
        }
    </style>
</head>
<body>
    <div class="header">
        <h3>ESP32 TRACKER</h3>
        <div>
            <span class="status disconnected">未连接</span>
            <button id="refreshBtn" class="btn btn-refresh">刷新轨迹</button>
        </div>
    </div>
    <div class="info-panel">
        <span id="distanceInfo">距离: -- km</span>
        <span id="statusInfo">状态: --</span>
    </div>
    <div id="map"></div>

    <script>
        // WGS84到GCJ-02坐标转换算法
        const transform = {
            X_PI: 3.14159265358979324 * 3000.0 / 180.0,
            PI: 3.1415926535897932384626,
            A: 6378245.0,
            EE: 0.00669342162296594323,
            
            wgs84ToGcj02(lng, lat) {
                let dlat = this.transformLat(lng - 105.0, lat - 35.0);
                let dlng = this.transformLon(lng - 105.0, lat - 35.0);
                let radlat = lat / 180.0 * this.PI;
                let magic = Math.sin(radlat);
                magic = 1 - this.EE * magic * magic;
                let sqrtmagic = Math.sqrt(magic);
                dlat = (dlat * 180.0) / ((this.A * (1 - this.EE)) / (magic * sqrtmagic) * this.PI);
                dlng = (dlng * 180.0) / (this.A / sqrtmagic * Math.cos(radlat) * this.PI);
                let mglat = lat + dlat;
                let mglng = lng + dlng;
                return [mglng, mglat];
            },
            
            transformLat(x, y) {
                let ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * Math.sqrt(Math.abs(x));
                ret += (20.0 * Math.sin(6.0 * x * this.PI) + 20.0 * Math.sin(2.0 * x * this.PI)) * 2.0 / 3.0;
                ret += (20.0 * Math.sin(y * this.PI) + 40.0 * Math.sin(y / 3.0 * this.PI)) * 2.0 / 3.0;
                ret += (160.0 * Math.sin(y / 12.0 * this.PI) + 320 * Math.sin(y * this.PI / 30.0)) * 2.0 / 3.0;
                return ret;
            },
            
            transformLon(x, y) {
                let ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * Math.sqrt(Math.abs(x));
                ret += (20.0 * Math.sin(6.0 * x * this.PI) + 20.0 * Math.sin(2.0 * x * this.PI)) * 2.0 / 3.0;
                ret += (20.0 * Math.sin(x * this.PI) + 40.0 * Math.sin(x / 3.0 * this.PI)) * 2.0 / 3.0;
                ret += (150.0 * Math.sin(x / 12.0 * this.PI) + 300.0 * Math.sin(x / 30.0 * this.PI)) * 2.0 / 3.0;
                return ret;
            }
        };
        
        // 初始化地图（北京默认中心点）
        const map = L.map('map').setView([39.9042, 116.4074], 12);
        
        // 高德地图瓦片源
        const gaodeTileLayer = L.tileLayer('https://webrd0{s}.is.autonavi.com/appmaptile?lang=zh_cn&size=1&scale=1&style=8&x={x}&y={y}&z={z}', {
            subdomains: '1234',
            attribution: '© 高德地图',
            maxZoom: 18,
            minZoom: 3
        }).addTo(map);
        
        // 轨迹图层组
        const trackLayer = L.layerGroup().addTo(map);
        
        // 当前轨迹线
        let currentTrack = null;
        let trackMarkers = [];

        function convertWgs84ToGcj02(points) {
            return points.map(point => {
                const [lng, lat] = transform.wgs84ToGcj02(point.lon, point.lat);
                return {
                    lon: lng,
                    lat: lat
                };
            });
        }
        
        function loadTrackData() {
            fetch('/trace')
                .then(response => {
                    if (!response.ok) {
                        throw new Error('网络请求失败');
                    }
                    return response.json();
                })
                .then(data => {
                    document.querySelector('.status').textContent = '已连接';
                    document.querySelector('.status').className = 'status connected';
                    document.getElementById('distanceInfo').textContent = `距离: ${(data.distance * 1000).toFixed(2)} 米`;
                    document.getElementById('statusInfo').textContent = `状态: ${data.in_track ? '记录中' : '未在记录'}`;
                    
                    clearTrack();
                    
                    const wgs84Points = data.trace;
                    if (wgs84Points.length > 0) {
                        const gcj02Points = convertWgs84ToGcj02(wgs84Points);
                        currentTrack = L.polyline(gcj02Points, {
                            color: '#ff4500',
                            weight: 4,
                            opacity: 0.8,
                            lineJoin: 'round',
                            className: 'track-line'
                        }).addTo(trackLayer);
                    }
                    if (points.length == 1) {
                        map.fitBounds(currentTrack.getBounds(), { padding: [50, 50] });
                    }
                })
                .catch(error => {
                    console.error('获取轨迹数据失败:', error);
                    document.querySelector('.status').textContent = '连接失败';
                    document.querySelector('.status').className = 'status disconnected';
                });
        }
        
        function clearTrack() {
            if (currentTrack) {
                trackLayer.removeLayer(currentTrack);
                currentTrack = null;
            }
            trackMarkers.forEach(marker => trackLayer.removeLayer(marker));
            trackMarkers = [];
        }
        
        loadTrackData();
        document.getElementById('refreshBtn').addEventListener('click', loadTrackData);
        setInterval(loadTrackData, 5000);
    </script>
</body>
</html>)";

#endif