<mxfile host="app.diagrams.net" agent="Kimi Code CLI">
  <diagram name="系统架构图" id="AlientekAiBox1_arch">
    <mxGraphModel dx="2163" dy="1200" grid="1" gridSize="10" guides="1" tooltips="1" connect="1" arrows="1" fold="1" page="1" pageScale="1" pageWidth="1600" pageHeight="900" math="0" shadow="0">
      <root>
        <mxCell id="0" />
        <mxCell id="1" parent="0" />
        <mxCell id="title" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=22;fontColor=#1a1a1a;" value="&lt;b&gt;AlientekAiBox1 V1.0 系统架构图&lt;/b&gt;" vertex="1">
          <mxGeometry height="40" width="400" x="600" y="20" as="geometry" />
        </mxCell>
        <mxCell id="layer_app" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#e1f5fe;strokeColor=#01579b;arcSize=6;" value="" vertex="1">
          <mxGeometry height="70" width="240" x="680" y="80" as="geometry" />
        </mxCell>
        <mxCell id="layer_app_label" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=12;fontStyle=1;fontColor=#01579b;" value="应用入口层 (App Entry)" vertex="1">
          <mxGeometry height="20" width="240" x="680" y="85" as="geometry" />
        </mxCell>
        <mxCell id="app_main" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff3e0;strokeColor=#e65100;fontSize=11;" value="&lt;b&gt;AlientekAiBox1.c&lt;/b&gt;&lt;br&gt;app_main() → app_init_task&lt;br&gt;Priority: 5 | Stack: 4KB" vertex="1">
          <mxGeometry height="40" width="160" x="720" y="105" as="geometry" />
        </mxCell>
        <mxCell id="layer_init" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#e8f5e9;strokeColor=#2e7d32;arcSize=6;" value="" vertex="1">
          <mxGeometry height="90" width="400" x="600" y="170" as="geometry" />
        </mxCell>
        <mxCell id="layer_init_label" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=12;fontStyle=1;fontColor=#2e7d32;" value="初始化层 (App Init) — 顺序初始化后自删除" vertex="1">
          <mxGeometry height="20" width="400" x="600" y="175" as="geometry" />
        </mxCell>
        <mxCell id="init_flow" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=11;" value="NVS Flash → Event System → WiFi Manager → LED Manager" vertex="1">
          <mxGeometry height="20" width="360" x="620" y="200" as="geometry" />
        </mxCell>
        <mxCell id="init_event" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#ffebee;strokeColor=#c62828;fontSize=10;" value="发布 APP_EVENT_INIT_DONE" vertex="1">
          <mxGeometry height="25" width="200" x="700" y="225" as="geometry" />
        </mxCell>
        <mxCell id="layer_core" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff8e1;strokeColor=#f57f17;arcSize=6;" value="" vertex="1">
          <mxGeometry height="220" width="1520" x="40" y="280" as="geometry" />
        </mxCell>
        <mxCell id="layer_core_label" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=13;fontStyle=1;fontColor=#f57f17;" value="核心服务层 (Core Services)" vertex="1">
          <mxGeometry height="20" width="300" x="55" y="285" as="geometry" />
        </mxCell>
        <mxCell id="event_box" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fce4ec;strokeColor=#880e4f;arcSize=4;" value="" vertex="1">
          <mxGeometry height="170" width="260" x="60" y="310" as="geometry" />
        </mxCell>
        <mxCell id="event_title" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=12;fontColor=#880e4f;" value="&lt;b&gt;app_event.c&lt;/b&gt; — 事件分发总线" vertex="1">
          <mxGeometry height="20" width="260" x="60" y="315" as="geometry" />
        </mxCell>
        <mxCell id="event_detail1" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=10;" value="• 优先队列 (32 slots)&lt;br&gt;• 最大 10 个监听器&lt;br&gt;• Task Priority: 4 | Stack: 4KB" vertex="1">
          <mxGeometry height="50" width="230" x="75" y="340" as="geometry" />
        </mxCell>
        <mxCell id="event_detail2" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=10;fontStyle=1;" value="WiFi Events | Prov Events | Key Events | Sys Events" vertex="1">
          <mxGeometry height="30" width="230" x="75" y="390" as="geometry" />
        </mxCell>
        <mxCell id="event_api" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#f8bbd0;strokeColor=#880e4f;fontSize=9;" value="post / post_simple / register_listener" vertex="1">
          <mxGeometry height="20" width="220" x="80" y="430" as="geometry" />
        </mxCell>
        <mxCell id="wifi_box" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#e3f2fd;strokeColor=#0d47a1;arcSize=4;" value="" vertex="1">
          <mxGeometry height="170" width="320" x="360" y="310" as="geometry" />
        </mxCell>
        <mxCell id="wifi_title" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=12;fontColor=#0d47a1;" value="&lt;b&gt;app_wifi.c&lt;/b&gt; — WiFi 管理器" vertex="1">
          <mxGeometry height="20" width="320" x="360" y="315" as="geometry" />
        </mxCell>
        <mxCell id="wifi_detail" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=10;" value="状态机: IDLE → CONNECTING / PROVISIONING → CONNECTED&lt;br&gt;配网超时: 120s | 重试延时: 5s&lt;br&gt;封装 SmartConfig Component" vertex="1">
          <mxGeometry height="50" width="290" x="375" y="340" as="geometry" />
        </mxCell>
        <mxCell id="wifi_api" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#bbdefb;strokeColor=#0d47a1;fontSize=9;" value="init | start | stop | start_provisioning | clear_config | get_ssid" vertex="1">
          <mxGeometry height="20" width="290" x="375" y="430" as="geometry" />
        </mxCell>
        <mxCell id="wifi_timer" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff;strokeColor=#0d47a1;fontSize=9;dashed=1;" value="配网超时定时器 (FreeRTOS Timer)" vertex="1">
          <mxGeometry height="20" width="290" x="375" y="455" as="geometry" />
        </mxCell>
        <mxCell id="led_box" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#f3e5f5;strokeColor=#4a148c;arcSize=4;" value="" vertex="1">
          <mxGeometry height="170" width="260" x="720" y="310" as="geometry" />
        </mxCell>
        <mxCell id="led_title" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=12;fontColor=#4a148c;" value="&lt;b&gt;app_led.c&lt;/b&gt; — LED 管理器" vertex="1">
          <mxGeometry height="20" width="260" x="720" y="315" as="geometry" />
        </mxCell>
        <mxCell id="led_detail" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=10;" value="GPIO4 系统状态指示灯&lt;br&gt;Task Priority: 1 (最低) | Stack: 2KB&lt;br&gt;监听事件: PROV_START / WIFI_CONNECTED / PROV_TIMEOUT" vertex="1">
          <mxGeometry height="60" width="230" x="735" y="340" as="geometry" />
        </mxCell>
        <mxCell id="led_states" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=10;fontStyle=1;" value="IDLE(1Hz) — PROVISIONING(5Hz) — PROV_TIMEOUT(1Hz)" vertex="1">
          <mxGeometry height="25" width="230" x="735" y="400" as="geometry" />
        </mxCell>
        <mxCell id="led_api" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#e1bee7;strokeColor=#4a148c;fontSize=9;" value="init | start | set_state" vertex="1">
          <mxGeometry height="20" width="220" x="740" y="430" as="geometry" />
        </mxCell>
        <mxCell id="led_driver_ref" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff;strokeColor=#4a148c;fontSize=9;dashed=1;" value="使用 led_driver.h + led_gpio.c" vertex="1">
          <mxGeometry height="20" width="220" x="740" y="455" as="geometry" />
        </mxCell>
        <mxCell id="led_drv_box" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#ede7f6;strokeColor=#311b92;arcSize=4;" value="" vertex="1">
          <mxGeometry height="170" width="240" x="1020" y="310" as="geometry" />
        </mxCell>
        <mxCell id="led_drv_title" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=12;fontColor=#311b92;" value="&lt;b&gt;LED 驱动抽象层&lt;/b&gt;" vertex="1">
          <mxGeometry height="20" width="240" x="1020" y="315" as="geometry" />
        </mxCell>
        <mxCell id="led_drv_detail" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=10;" value="&lt;b&gt;led_driver.h&lt;/b&gt;&lt;br&gt;面向对象驱动接口 (C中的虚函数表)&lt;br&gt;led_device_t + led_driver_iface_t&lt;br&gt;&lt;br&gt;&lt;b&gt;led_gpio.c&lt;/b&gt;&lt;br&gt;GPIO 驱动实现 (esp_driver_gpio)&lt;br&gt;支持 active_low / active_high" vertex="1">
          <mxGeometry height="100" width="210" x="1035" y="340" as="geometry" />
        </mxCell>
        <mxCell id="led_drv_api" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#d1c4e9;strokeColor=#311b92;fontSize=9;" value="init | on | off | toggle | is_on" vertex="1">
          <mxGeometry height="20" width="210" x="1035" y="445" as="geometry" />
        </mxCell>
        <mxCell id="key_box" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff;strokeColor=#607d8b;fontSize=10;dashed=1;dashPattern=1 1;" value="&lt;b&gt;按键管理器 (预留)&lt;/b&gt;&lt;br&gt;app_event.h 已定义 KEY0/KEY1 事件&lt;br&gt;SHORT / LONG / DOUBLE" vertex="1">
          <mxGeometry height="80" width="230" x="1300" y="310" as="geometry" />
        </mxCell>
        <mxCell id="layer_sc" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#e0f7fa;strokeColor=#006064;arcSize=6;" value="" vertex="1">
          <mxGeometry height="280" width="1520" x="40" y="520" as="geometry" />
        </mxCell>
        <mxCell id="layer_sc_label" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=13;fontStyle=1;fontColor=#006064;" value="SmartConfig 组件层 (components/smartconfig_component) — 面向对象 / 依赖注入 / 观察者模式" vertex="1">
          <mxGeometry height="20" width="700" x="55" y="525" as="geometry" />
        </mxCell>
        <mxCell id="sc_core" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#b2ebf2;strokeColor=#006064;arcSize=4;" value="" vertex="1">
          <mxGeometry height="230" width="600" x="500" y="550" as="geometry" />
        </mxCell>
        <mxCell id="sc_core_title" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=12;fontColor=#006064;" value="&lt;b&gt;smartconfig_component.c — 核心组件&lt;/b&gt; | sc_component_t (OO in C)" vertex="1">
          <mxGeometry height="20" width="600" x="500" y="555" as="geometry" />
        </mxCell>
        <mxCell id="sc_core_detail" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=10;" value="生命周期: create → init → start/stop → destroy&lt;br&gt;状态机: IDLE → STARTING → SCANNING → FOUND_CHANNEL → GETTING_SSID_PSWD → CONNECTING → CONNECTED/FAILED/STOPPED&lt;br&gt;同步: Mutex + EventGroup | 观察者: 5 slots | Task Priority: 3 | Stack: 4KB | auto_save_wifi / auto_reconnect" vertex="1">
          <mxGeometry height="60" width="570" x="515" y="580" as="geometry" />
        </mxCell>
        <mxCell id="sc_core_api" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#80deea;strokeColor=#006064;fontSize=9;" value="create | init | start | stop | attach_observer | connect_saved | has_saved_wifi | clear_wifi | get_status" vertex="1">
          <mxGeometry height="22" width="570" x="515" y="640" as="geometry" />
        </mxCell>
        <mxCell id="sc_internal" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#e0f2f1;strokeColor=#006064;fontSize=9;" value="&lt;b&gt;内部通信接口&lt;/b&gt; (smartconfig_component_internal.h)&lt;br&gt;sc_internal_on_got_ssid_pswd | sc_internal_on_driver_status | sc_internal_on_wifi_connected | sc_internal_on_wifi_disconnected" vertex="1">
          <mxGeometry height="35" width="570" x="515" y="670" as="geometry" />
        </mxCell>
        <mxCell id="sc_events" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#e0f2f1;strokeColor=#006064;fontSize=9;" value="内部事件: START | STOP | CONNECTED | FAILED | TIMEOUT | GOT_SSID_PSWD" vertex="1">
          <mxGeometry height="22" width="570" x="515" y="712" as="geometry" />
        </mxCell>
        <mxCell id="sc_deps" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#4dd0e1;strokeColor=#006064;fontSize=9;fontStyle=1;" value="依赖注入: driver_iface_t + storage_iface_t + wifi_iface_t" vertex="1">
          <mxGeometry height="22" width="570" x="515" y="742" as="geometry" />
        </mxCell>
        <mxCell id="sc_driver" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff3e0;strokeColor=#e65100;arcSize=4;" value="" vertex="1">
          <mxGeometry height="230" width="200" x="60" y="550" as="geometry" />
        </mxCell>
        <mxCell id="sc_driver_title" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=12;fontColor=#e65100;" value="&lt;b&gt;配网驱动&lt;/b&gt;" vertex="1">
          <mxGeometry height="20" width="200" x="60" y="555" as="geometry" />
        </mxCell>
        <mxCell id="sc_esptouch" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=10;" value="&lt;b&gt;sc_driver_esptouch.c&lt;/b&gt;&lt;br&gt;策略: SC_TYPE_ESPTOUCH&lt;br&gt;状态: SCANNING → FOUND_CHANNEL → GOT_SSID_PSWD&lt;br&gt;事件: SC_EVENT (ESP-IDF SmartConfig)&lt;br&gt;&lt;br&gt;API:&lt;br&gt;init / start / stop / get_status&lt;br&gt;&lt;br&gt;Context:&lt;br&gt;esptouch_ctx_t (component ref)" vertex="1">
          <mxGeometry height="160" width="170" x="75" y="575" as="geometry" />
        </mxCell>
        <mxCell id="sc_storage" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#f1f8e9;strokeColor=#558b2f;arcSize=4;" value="" vertex="1">
          <mxGeometry height="230" width="180" x="290" y="550" as="geometry" />
        </mxCell>
        <mxCell id="sc_storage_title" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=12;fontColor=#558b2f;" value="&lt;b&gt;存储后端&lt;/b&gt;" vertex="1">
          <mxGeometry height="20" width="180" x="290" y="555" as="geometry" />
        </mxCell>
        <mxCell id="sc_nvs" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=10;" value="&lt;b&gt;sc_storage_nvs.c&lt;/b&gt;&lt;br&gt;后端: NVS Flash&lt;br&gt;Namespace: &quot;smartconfig&quot;&lt;br&gt;Key: &quot;wifi_config&quot;&lt;br&gt;&lt;br&gt;API:&lt;br&gt;init / load / save / clear / has_config&lt;br&gt;&lt;br&gt;Context:&lt;br&gt;nvs_storage_ctx_t (handle)" vertex="1">
          <mxGeometry height="160" width="160" x="300" y="575" as="geometry" />
        </mxCell>
        <mxCell id="sc_wifi" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#e8eaf6;strokeColor=#283593;arcSize=4;" value="" vertex="1">
          <mxGeometry height="230" width="200" x="1130" y="550" as="geometry" />
        </mxCell>
        <mxCell id="sc_wifi_title" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=12;fontColor=#283593;" value="&lt;b&gt;WiFi 管理&lt;/b&gt;" vertex="1">
          <mxGeometry height="20" width="200" x="1130" y="555" as="geometry" />
        </mxCell>
        <mxCell id="sc_wifi_esp32" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=10;" value="&lt;b&gt;sc_wifi_esp32.c&lt;/b&gt;&lt;br&gt;平台: ESP32 STA Mode&lt;br&gt;事件: WIFI_EVENT + IP_EVENT&lt;br&gt;IP_EVENT_STA_GOT_IP → connected&lt;br&gt;&lt;br&gt;API:&lt;br&gt;init / connect / disconnect / is_connected&lt;br&gt;&lt;br&gt;Context:&lt;br&gt;esp32_wifi_ctx_t (component ref)" vertex="1">
          <mxGeometry height="160" width="170" x="1145" y="575" as="geometry" />
        </mxCell>
        <mxCell id="layer_hal" parent="1" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#eceff1;strokeColor=#455a64;arcSize=6;" value="" vertex="1">
          <mxGeometry height="70" width="1520" x="40" y="820" as="geometry" />
        </mxCell>
        <mxCell id="layer_hal_label" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=13;fontStyle=1;fontColor=#455a64;" value="硬件抽象层 (ESP-IDF HAL)" vertex="1">
          <mxGeometry height="20" width="300" x="55" y="825" as="geometry" />
        </mxCell>
        <mxCell id="hal_items" parent="1" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=10;" value="FreeRTOS (Tasks, Timers, Queues, EventGroups, Mutex)  |  esp_wifi  |  esp_smartconfig  |  nvs_flash  |  esp_driver_gpio  |  esp_netif  |  wpa_supplicant" vertex="1">
          <mxGeometry height="30" width="1480" x="60" y="850" as="geometry" />
        </mxCell>
        <mxCell id="e1" edge="1" parent="1" source="app_main" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;strokeColor=#424242;" target="layer_init">
          <mxGeometry relative="1" as="geometry" />
        </mxCell>
        <mxCell id="e2" edge="1" parent="1" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;strokeColor=#c62828;dashed=1;dashPattern=1 1;" target="event_box">
          <mxGeometry relative="1" as="geometry">
            <Array as="points">
              <mxPoint x="190" y="290" />
            </Array>
            <mxPoint x="520" y="290" as="sourcePoint" />
          </mxGeometry>
        </mxCell>
        <mxCell id="e3" edge="1" parent="1" source="layer_init" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;strokeColor=#0d47a1;" target="wifi_box">
          <mxGeometry relative="1" as="geometry">
            <Array as="points">
              <mxPoint x="800" y="290" />
              <mxPoint x="520" y="290" />
            </Array>
          </mxGeometry>
        </mxCell>
        <mxCell id="e4" edge="1" parent="1" source="layer_init" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;strokeColor=#4a148c;" target="led_box">
          <mxGeometry relative="1" as="geometry">
            <Array as="points">
              <mxPoint x="800" y="290" />
              <mxPoint x="850" y="290" />
            </Array>
          </mxGeometry>
        </mxCell>
        <mxCell id="e5" edge="1" parent="1" source="wifi_box" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;strokeColor=#c62828;exitX=0;exitY=0.5;exitDx=0;exitDy=0;entryX=1;entryY=0.5;entryDx=0;entryDy=0;fontSize=9;" target="event_box" value="发布事件">
          <mxGeometry relative="1" as="geometry" />
        </mxCell>
        <mxCell id="e6" edge="1" parent="1" source="led_box" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;strokeColor=#c62828;exitX=0.537;exitY=1.003;exitDx=0;exitDy=0;fontSize=9;exitPerimeter=0;" target="event_box" value="订阅事件">
          <mxGeometry relative="1" x="0.1723" as="geometry">
            <mxPoint as="offset" />
            <Array as="points">
              <mxPoint x="860" y="510" />
              <mxPoint x="170" y="510" />
            </Array>
            <mxPoint x="600" y="482.49" as="sourcePoint" />
            <mxPoint x="170" y="487.49" as="targetPoint" />
          </mxGeometry>
        </mxCell>
        <mxCell id="e7" edge="1" parent="1" source="led_box" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;strokeColor=#311b92;" target="led_drv_box">
          <mxGeometry relative="1" as="geometry" />
        </mxCell>
        <mxCell id="e8" edge="1" parent="1" source="led_drv_box" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;strokeColor=#455a64;" target="layer_hal">
          <mxGeometry relative="1" as="geometry">
            <Array as="points">
              <mxPoint x="1140" y="530" />
              <mxPoint x="1420" y="530" />
            </Array>
          </mxGeometry>
        </mxCell>
        <mxCell id="e9" edge="1" parent="1" source="wifi_box" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;strokeColor=#006064;exitX=0.5;exitY=1;exitDx=0;exitDy=0;entryX=0.5;entryY=0;entryDx=0;entryDy=0;fontSize=9;" target="sc_core" value="封装调用">
          <mxGeometry relative="1" as="geometry" />
        </mxCell>
        <mxCell id="e10" edge="1" parent="1" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;strokeColor=#e65100;fontSize=9;" value="driver_iface_t">
          <mxGeometry relative="1" as="geometry">
            <Array as="points">
              <mxPoint x="770" y="790" />
              <mxPoint x="160" y="790" />
            </Array>
            <mxPoint x="770" y="780" as="sourcePoint" />
            <mxPoint x="160" y="780" as="targetPoint" />
          </mxGeometry>
        </mxCell>
        <mxCell id="e11" edge="1" parent="1" source="sc_core" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;strokeColor=#558b2f;exitX=0;exitY=0.5;exitDx=0;exitDy=0;entryX=1;entryY=0.5;entryDx=0;entryDy=0;fontSize=9;" target="sc_storage" value="storage_iface_t">
          <mxGeometry relative="1" as="geometry" />
        </mxCell>
        <mxCell id="e12" edge="1" parent="1" source="sc_core" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;strokeColor=#283593;exitX=1;exitY=0.5;exitDx=0;exitDy=0;entryX=0;entryY=0.5;entryDx=0;entryDy=0;fontSize=9;" target="sc_wifi" value="wifi_iface_t">
          <mxGeometry relative="1" as="geometry" />
        </mxCell>
        <mxCell id="e13" edge="1" parent="1" source="sc_driver" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;strokeColor=#455a64;" target="layer_hal">
          <mxGeometry relative="1" as="geometry" />
        </mxCell>
        <mxCell id="e14" edge="1" parent="1" source="sc_storage" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;strokeColor=#455a64;" target="layer_hal">
          <mxGeometry relative="1" as="geometry" />
        </mxCell>
        <mxCell id="e15" edge="1" parent="1" source="sc_wifi" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;strokeColor=#455a64;" target="layer_hal">
          <mxGeometry relative="1" as="geometry" />
        </mxCell>
        <mxCell id="note" parent="1" style="shape=note;whiteSpace=wrap;html=1;backgroundOutline=1;darkOpacity=0.05;fillColor=#fff9c4;strokeColor=#f9a825;fontSize=10;" value="&lt;b&gt;设计模式总结&lt;/b&gt;&lt;br&gt;• 依赖注入 (DI): SmartConfig 组件通过接口注入 Driver / Storage / WiFi&lt;br&gt;• 观察者模式: sc_component_attach_observer() 支持最多 5 个观察者&lt;br&gt;• 事件驱动: app_event 作为全局事件总线，解耦 WiFi 与 LED&lt;br&gt;• 面向对象 C: LED Device 和 SC Component 均使用 struct + vtable 模拟类" vertex="1">
          <mxGeometry height="100" width="240" x="1300" y="410" as="geometry" />
        </mxCell>
        <mxCell id="WgEI_d5JpjiV3kIoHuXP-3" edge="1" parent="1" source="layer_core_label" style="edgeStyle=orthogonalEdgeStyle;rounded=0;orthogonalLoop=1;jettySize=auto;html=1;strokeColor=#c62828;exitX=0.338;exitY=1.089;exitDx=0;exitDy=0;fontSize=9;dashed=1;dashPattern=1 1;exitPerimeter=0;" target="key_box" value="事件总线 (预留扩展)">
          <mxGeometry relative="1" x="0.4554" as="geometry">
            <mxPoint as="offset" />
            <Array as="points">
              <mxPoint x="156" y="265" />
              <mxPoint x="1410" y="265" />
            </Array>
            <mxPoint x="430" y="280" as="sourcePoint" />
            <mxPoint x="1410" y="235" as="targetPoint" />
          </mxGeometry>
        </mxCell>
      </root>
    </mxGraphModel>
  </diagram>
</mxfile>
