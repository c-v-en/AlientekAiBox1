import re

new_diagram2 = '''    <diagram id="tdDFiXi_28T6un0gf8xL" name="双核通信架构">
        <mxGraphModel dx="1434" dy="774" grid="1" gridSize="10" guides="1" tooltips="1" connect="1" arrows="1" fold="1" page="1" pageScale="1" pageWidth="1200" pageHeight="900" background="#ffffff" math="0" shadow="0">
            <root>
                <mxCell id="0"/>
                <mxCell id="1" parent="0"/>

                <!-- Title -->
                <mxCell id="2" value="&lt;b&gt;ESP32-S3 双核通信架构与线程安全设计&lt;/b&gt;" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=20;fontColor=#1a1a1a;" vertex="1" parent="1">
                    <mxGeometry x="200" y="20" width="800" height="40" as="geometry"/>
                </mxCell>

                <!-- ==================== Core 0 Container ==================== -->
                <mxCell id="10" value="" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#e3f2fd;strokeColor=#1565c0;arcSize=4;" vertex="1" parent="1">
                    <mxGeometry x="40" y="90" width="480" height="720" as="geometry"/>
                </mxCell>
                <mxCell id="11" value="&lt;b&gt;Core 0 - 网络协议栈域 (Network Domain)&lt;/b&gt;" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=14;fontColor=#1565c0;" vertex="1" parent="1">
                    <mxGeometry x="40" y="95" width="480" height="25" as="geometry"/>
                </mxCell>

                <!-- System Tasks -->
                <mxCell id="12" value="" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#eceff1;strokeColor=#546e7a;arcSize=4;" vertex="1" parent="1">
                    <mxGeometry x="60" y="130" width="440" height="210" as="geometry"/>
                </mxCell>
                <mxCell id="13" value="&lt;b&gt;ESP-IDF 系统任务（已绑定或建议绑定 Core0）&lt;/b&gt;" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=11;fontColor=#37474f;" vertex="1" parent="1">
                    <mxGeometry x="60" y="135" width="440" height="20" as="geometry"/>
                </mxCell>
                <mxCell id="14" value="WiFi Task&lt;br&gt;&lt;font color=#2e7d32&gt;✓ CONFIG_ESP_WIFI_TASK_PINNED_TO_CORE_0&lt;/font&gt;" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff;strokeColor=#1565c0;fontSize=10;" vertex="1" parent="1">
                    <mxGeometry x="80" y="165" width="190" height="50" as="geometry"/>
                </mxCell>
                <mxCell id="15" value="ESP Timer Task&lt;br&gt;&lt;font color=#2e7d32&gt;✓ CONFIG_ESP_TIMER_TASK_AFFINITY_CPU0&lt;/font&gt;" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff;strokeColor=#1565c0;fontSize=10;" vertex="1" parent="1">
                    <mxGeometry x="290" y="165" width="190" height="50" as="geometry"/>
                </mxCell>
                <mxCell id="16" value="TCP/IP Task (lwip)&lt;br&gt;&lt;font color=#e65100&gt;➜ 建议绑定: CONFIG_LWIP_TCPIP_TASK_AFFINITY_CPU0&lt;/font&gt;" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff3e0;strokeColor=#e65100;fontSize=10;dashed=1;" vertex="1" parent="1">
                    <mxGeometry x="80" y="230" width="190" height="50" as="geometry"/>
                </mxCell>
                <mxCell id="17" value="MQTT Task (espressif__mqtt)&lt;br&gt;&lt;font color=#2e7d32&gt;✓ MQTT_TASK_CORE = 0&lt;/font&gt;" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff;strokeColor=#1565c0;fontSize=10;" vertex="1" parent="1">
                    <mxGeometry x="290" y="230" width="190" height="50" as="geometry"/>
                </mxCell>
                <mxCell id="18" value="WiFi/BT/TCPIP 中断与回调运行于 Core0 上下文，避免频繁核间 IPC" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=9;fontColor=#546e7a;fontStyle=2;" vertex="1" parent="1">
                    <mxGeometry x="60" y="290" width="440" height="30" as="geometry"/>
                </mxCell>

                <!-- User Tasks Core0 -->
                <mxCell id="20" value="" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff;strokeColor=#1565c0;arcSize=4;" vertex="1" parent="1">
                    <mxGeometry x="60" y="360" width="440" height="260" as="geometry"/>
                </mxCell>
                <mxCell id="21" value="&lt;b&gt;用户任务（绑定 Core0）&lt;/b&gt;" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=11;fontColor=#1565c0;" vertex="1" parent="1">
                    <mxGeometry x="60" y="365" width="440" height="20" as="geometry"/>
                </mxCell>
                <mxCell id="22" value="&lt;b&gt;wifi_ctrl_task&lt;/b&gt;&lt;br&gt;Priority: 4 | Stack: 2KB | Core: 0&lt;br&gt;• 消费 wifi_cmd_queue&lt;br&gt;• 执行实际 WiFi API 调用&lt;br&gt;• 禁止被其他核直接调用" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff3e0;strokeColor=#e65100;fontSize=10;" vertex="1" parent="1">
                    <mxGeometry x="80" y="395" width="400" height="90" as="geometry"/>
                </mxCell>
                <mxCell id="23" value="&lt;b&gt;sc_task (SmartConfig)&lt;/b&gt;&lt;br&gt;Priority: 3 | Stack: 4KB | Core: 0&lt;br&gt;• 配网状态机 (EventGroup + Mutex)&lt;br&gt;• 观察者回调发布事件到 Event Bus&lt;br&gt;• xTaskCreatePinnedToCore(..., 0)" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff3e0;strokeColor=#e65100;fontSize=10;" vertex="1" parent="1">
                    <mxGeometry x="80" y="500" width="400" height="90" as="geometry"/>
                </mxCell>

                <!-- I2C Mutex Hint Core0 -->
                <mxCell id="25" value="&lt;b&gt;跨核共享资源保护&lt;/b&gt;&lt;br&gt;I2C Bus 新增 Mutex: 任何核调用 read_reg/write_reg 前必须 xSemaphoreTake/Give&lt;br&gt;当前主要使用方在 Core1，但 Mutex 确保未来扩展安全" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#ffebee;strokeColor=#c62828;fontSize=10;" vertex="1" parent="1">
                    <mxGeometry x="60" y="640" width="440" height="60" as="geometry"/>
                </mxCell>
                <mxCell id="26" value="所有 API 调用必须收敛到绑定核的任务内执行；其他核仅通过 Queue/Event 通信" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=9;fontColor=#c62828;fontStyle=1;" vertex="1" parent="1">
                    <mxGeometry x="60" y="710" width="440" height="30" as="geometry"/>
                </mxCell>

                <!-- ==================== Core 1 Container ==================== -->
                <mxCell id="30" value="" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#e8f5e9;strokeColor=#2e7d32;arcSize=4;" vertex="1" parent="1">
                    <mxGeometry x="680" y="90" width="480" height="720" as="geometry"/>
                </mxCell>
                <mxCell id="31" value="&lt;b&gt;Core 1 - 交互与外设域 (UI &amp;amp; IO Domain)&lt;/b&gt;" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=14;fontColor=#2e7d32;" vertex="1" parent="1">
                    <mxGeometry x="680" y="95" width="480" height="25" as="geometry"/>
                </mxCell>

                <!-- User Tasks Core1 -->
                <mxCell id="32" value="" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff;strokeColor=#2e7d32;arcSize=4;" vertex="1" parent="1">
                    <mxGeometry x="700" y="130" width="440" height="330" as="geometry"/>
                </mxCell>
                <mxCell id="33" value="&lt;b&gt;用户任务（绑定 Core1）&lt;/b&gt;" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=11;fontColor=#2e7d32;" vertex="1" parent="1">
                    <mxGeometry x="700" y="135" width="440" height="20" as="geometry"/>
                </mxCell>
                <mxCell id="34" value="&lt;b&gt;app_event_task&lt;/b&gt;&lt;br&gt;Priority: 4 | Stack: 4KB | Core: 1&lt;br&gt;• 全局事件分发中枢 (32 slots 优先队列)&lt;br&gt;• 接收 Core0 事件，分发到 Core1 监听器&lt;br&gt;• LED / UI 事件在此闭环" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#e1f5fe;strokeColor=#01579b;fontSize=10;" vertex="1" parent="1">
                    <mxGeometry x="720" y="165" width="400" height="80" as="geometry"/>
                </mxCell>
                <mxCell id="35" value="&lt;b&gt;lvgl_task&lt;/b&gt;&lt;br&gt;Priority: 4 | Stack: 8KB | Core: 1&lt;br&gt;• 唯一允许调用 lv_xxx() 的任务&lt;br&gt;• 周期: lv_timer_handler()&lt;br&gt;• 其他任务通过消息队列/标志位通知 UI 更新" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#f3e5f5;strokeColor=#7b1fa2;fontSize=10;" vertex="1" parent="1">
                    <mxGeometry x="720" y="255" width="400" height="80" as="geometry"/>
                </mxCell>
                <mxCell id="36" value="&lt;b&gt;xl9555_key_task&lt;/b&gt;&lt;br&gt;Priority: 2 | Stack: 2KB | Core: 1&lt;br&gt;• INT 中断 - Queue - 消抖 - 发布 KEY0/KEY1 事件&lt;br&gt;• 绝不直接调用 WiFi API" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff8e1;strokeColor=#f57f17;fontSize=10;" vertex="1" parent="1">
                    <mxGeometry x="720" y="345" width="400" height="60" as="geometry"/>
                </mxCell>
                <mxCell id="37" value="&lt;b&gt;led_task&lt;/b&gt;&lt;br&gt;Priority: 1 | Stack: 2KB | Core: 1&lt;br&gt;• 监听 WiFi/Prov 事件，驱动 GPIO4 闪烁&lt;br&gt;• 纯本地 Core1 闭环" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff8e1;strokeColor=#f57f17;fontSize=10;" vertex="1" parent="1">
                    <mxGeometry x="720" y="415" width="400" height="50" as="geometry"/>
                </mxCell>

                <!-- Shared Resources Core1 -->
                <mxCell id="40" value="" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fffde7;strokeColor=#f9a825;arcSize=4;" vertex="1" parent="1">
                    <mxGeometry x="700" y="480" width="440" height="140" as="geometry"/>
                </mxCell>
                <mxCell id="41" value="&lt;b&gt;共享资源与约束&lt;/b&gt;" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=11;fontColor=#f57f17;" vertex="1" parent="1">
                    <mxGeometry x="700" y="485" width="440" height="20" as="geometry"/>
                </mxCell>
                <mxCell id="42" value="&lt;b&gt;I2C Bus (i2c_bus_component)&lt;/b&gt;&lt;br&gt;SCL=45 SDA=48 | 400kHz&lt;br&gt;🔒 Mutex 保护: xSemaphoreTake/Give 包裹 read_reg / write_reg&lt;br&gt;当前调用方: xl9555_key_task (Core1) + app_init_task (已删除)" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#ffebee;strokeColor=#c62828;fontSize=10;" vertex="1" parent="1">
                    <mxGeometry x="720" y="510" width="400" height="70" as="geometry"/>
                </mxCell>
                <mxCell id="43" value="Event Bus 本地消费: LED / LVGL 监听器在 Core1 内闭环，不跨核回调" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=9;fontColor=#2e7d32;fontStyle=1;" vertex="1" parent="1">
                    <mxGeometry x="700" y="590" width="440" height="25" as="geometry"/>
                </mxCell>
                <mxCell id="44" value="LVGL API 铁律: 只有 lvgl_task 可直接调用 lv_xxx()&lt;br&gt;网络事件 - 标志位/Queue - lvgl_task 轮询更新" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=9;fontColor=#7b1fa2;fontStyle=1;" vertex="1" parent="1">
                    <mxGeometry x="700" y="630" width="440" height="30" as="geometry"/>
                </mxCell>
                <mxCell id="45" value="UI / IO 时序敏感任务与网络协议栈物理隔离，避免 WiFi 中断抖动影响 LCD 刷新" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=9;fontColor=#2e7d32;fontStyle=2;" vertex="1" parent="1">
                    <mxGeometry x="700" y="670" width="440" height="30" as="geometry"/>
                </mxCell>

                <!-- ==================== Middle Communication ==================== -->
                <mxCell id="50" value="" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff;strokeColor=#424242;arcSize=4;dashed=1;dashPattern=1 1;" vertex="1" parent="1">
                    <mxGeometry x="540" y="130" width="120" height="640" as="geometry"/>
                </mxCell>
                <mxCell id="51" value="&lt;b&gt;跨核通信&lt;/b&gt;" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=11;fontColor=#424242;" vertex="1" parent="1">
                    <mxGeometry x="540" y="135" width="120" height="20" as="geometry"/>
                </mxCell>

                <!-- Command Queue labels -->
                <mxCell id="53" value="&lt;b&gt;命令队列&lt;/b&gt;&lt;br&gt;&lt;font color=#e65100&gt;Cmd Queue&lt;/font&gt;" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=10;fontColor=#424242;" vertex="1" parent="1">
                    <mxGeometry x="540" y="280" width="120" height="40" as="geometry"/>
                </mxCell>
                <mxCell id="54" value="Core1 - Core0&lt;br&gt;xQueueSend&quot; style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=9;fontColor=#e65100;" vertex="1" parent="1">
                    <mxGeometry x="540" y="320" width="120" height="30" as="geometry"/>
                </mxCell>
                <mxCell id="55" value="KEY0_SHORT&lt;br&gt;app_wifi_request_prov()&lt;br&gt;↓ xQueueSend&lt;br&gt;wifi_ctrl_task 执行" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=9;fontColor=#424242;" vertex="1" parent="1">
                    <mxGeometry x="545" y="360" width="110" height="80" as="geometry"/>
                </mxCell>

                <!-- Event Bus labels -->
                <mxCell id="60" value="&lt;b&gt;事件总线&lt;/b&gt;&lt;br&gt;&lt;font color=#1565c0&gt;Event Bus&lt;/font&gt;" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=10;fontColor=#424242;" vertex="1" parent="1">
                    <mxGeometry x="540" y="480" width="120" height="40" as="geometry"/>
                </mxCell>
                <mxCell id="61" value="Core0 - Core1&lt;br&gt;app_event_post()" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=9;fontColor=#1565c0;" vertex="1" parent="1">
                    <mxGeometry x="540" y="520" width="120" height="30" as="geometry"/>
                </mxCell>
                <mxCell id="62" value="sc_status_callback&lt;br&gt;↓ app_event_post&lt;br&gt;PROV_START&lt;br&gt;WIFI_CONNECTED&lt;br&gt;↓ Core1 分发&lt;br&gt;LED / LCD 更新" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=top;whiteSpace=wrap;rounded=0;fontSize=9;fontColor=#424242;" vertex="1" parent="1">
                    <mxGeometry x="545" y="560" width="110" height="110" as="geometry"/>
                </mxCell>

                <!-- Arrows in middle (simplified shapes) -->
                <mxCell id="70" value="➤" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=40;fontColor=#e65100;" vertex="1" parent="1">
                    <mxGeometry x="560" y="250" width="80" height="50" as="geometry"/>
                </mxCell>
                <mxCell id="71" value="➤" style="text;html=1;strokeColor=none;fillColor=none;align=center;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=40;fontColor=#1565c0;rotation=180;" vertex="1" parent="1">
                    <mxGeometry x="560" y="600" width="80" height="50" as="geometry"/>
                </mxCell>

                <!-- ==================== Bottom Rules ==================== -->
                <mxCell id="80" value="" style="rounded=1;whiteSpace=wrap;html=1;fillColor=#fff9c4;strokeColor=#f9a825;arcSize=4;" vertex="1" parent="1">
                    <mxGeometry x="40" y="830" width="1120" height="60" as="geometry"/>
                </mxCell>
                <mxCell id="81" value="&lt;b&gt;双核解耦铁律&lt;/b&gt;&lt;br&gt;1. API 不跨核 - Core0 API 只能在 Core0 任务内执行；Core1 同理。&lt;br&gt;2. 异步不阻塞 - Core1 向 Core0 发命令用 xQueueSend，不等待返回值（状态由 Event 异步返回）。&lt;br&gt;3. 事件单向广播 - app_event_component 作为只读状态总线，任何核可发布，任何核可订阅。&lt;br&gt;4. UI 单入口 - 只有 lvgl_task 可直接调用 LVGL API，其他任务仅能发消息。" style="text;html=1;strokeColor=none;fillColor=none;align=left;verticalAlign=middle;whiteSpace=wrap;rounded=0;fontSize=10;fontColor=#424242;" vertex="1" parent="1">
                    <mxGeometry x="55" y="835" width="1090" height="50" as="geometry"/>
                </mxCell>

            </root>
        </mxGraphModel>
    </diagram>'''

with open('AlientekAiBox1_V1.0.drawio', 'r', encoding='utf-8') as f:
    content = f.read()

pattern = r'(\s*<diagram id="tdDFiXi_28T6un0gf8xL" name="双核通信架构">).*?(</diagram>)'
replacement = new_diagram2

new_content = re.sub(pattern, replacement, content, flags=re.DOTALL)

if new_content == content:
    print("WARNING: Replacement did not happen!")
else:
    with open('AlientekAiBox1_V1.0.drawio', 'w', encoding='utf-8') as f:
        f.write(new_content)
    print("Replacement successful. New length:", len(new_content))
