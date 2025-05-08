console.log("initing websockets")

let websocket = null
let draw_stack_interval = null

const open_connection = (roomCode) => {
    if (check_connection()) {
        console.log("websocket is already open")
        return
    }
    const is_production = window.location.hostname !== "localhost"
    const default_url = `ws://localhost:8080/ws?room=${roomCode}`
    const prod_url = `wss://wasm-draw.art/ws?room=${roomCode}`
    const url = is_production ? prod_url : default_url
    try {
        websocket = new WebSocket(url)
    } catch (err) {
        websocket = null
        console.error(err)
    }

    websocket.onopen = () => {
        console.log("websocket connection established", url)
        showRoomCode(roomCode)
        change_join_button("join-room", true)
        const processDrawStack = async () => {
            if (check_connection()) {
                try {
                    serialized_stack = processDrawCommands()
                    let msg = {
                        type: "draw",
                        payload: serialized_stack
                    }
                    websocket.send(JSON.stringify(msg))
                } catch (err) {
                    console.error("error getting or processing draw stack", err)
                }
            }
            draw_stack_interval = setTimeout(processDrawStack, 50);
        }
        processDrawStack()
    }

    websocket.onclose = () => {
        console.log("websocket connection closed")
        hideRoomCode()
        change_join_button("join-room", false)
        clearTimeout(draw_stack_interval)
        draw_stack_interval = null
        websocket = null
    }

    websocket.onerror = (error) => {
        console.log("error: ", error)
    }

    websocket.onmessage = (event) => {
        let message = JSON.parse(event.data)
        if (message.type === "draw") {
            for (const obj of message.payload) {
                const startX = obj.startX
                const startY = obj.startY
                const endX = obj.endX
                const endY = obj.endY
                const type = obj.type

                if (type === 0) {
                    Module.ccall(
                        "drawLine",
                        null,
                        ["number", "number", "number", "number", "number", "number"],
                        [startX, startY, endX, endY, 0, 0]
                    )
                }
            }
        } else if (message.type === "clear-canvas") {
            Module.ccall("clearCanvas", null, [])
        }
    }
}

const check_connection = () => {
    if (websocket && websocket.readyState === websocket.OPEN)
        return true
    else
        return false
}

const close_connection = () => {
    if (check_connection()) {
        websocket.close(1000, "closing connection normally")
    } else {
        console.log("websocket is already closed or not initialized")
    }
    websocket = null
}

const send_message = (payload = "hello") => {
    if (check_connection()) {
        websocket.send(payload)
        console.log("client sent message: ", payload)
    } else {
        console.error(`websocket is not open, message "${payload}" not sent`)
    }
}

