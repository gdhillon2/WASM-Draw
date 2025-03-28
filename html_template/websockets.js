console.log("initing websockets")

let websocket = null
let draw_stack_interval = null

const open_connection = () => {
    if (check_connection()) {
        console.log("websocket is already open")
        return
    }

    websocket = new WebSocket("ws://localhost:3000/ws")

    websocket.onopen = () => {
        console.log("websocket connection established")
        draw_stack_interval = setInterval(() => {
            if (check_connection() && canvas_sync) {
                draw_stack = canvas_sync.get_rest_of_draw_stack()
                if (draw_stack.size() > 0) {
                    let serialized_stack = []
                    for (let i = 0; i < draw_stack.size(); ++i) {
                        let cmd = draw_stack.get(i)
                        serialized_stack.push({
                            startX: cmd.startX,
                            startY: cmd.startY,
                            endX: cmd.endX,
                            endY: cmd.endY,
                            type: cmd.type
                        })
                    }
                    websocket.send(JSON.stringify(serialized_stack))
                }
            }
        }, 500);
        
    }

    websocket.onclose = () => {
        console.log("websocket connection closed")
        clearInterval(draw_stack_interval)
        draw_stack_interval = null
    }

    websocket.onerror = (error) => {
        console.log("error: ", error)
    }

    websocket.onmessage = (event) => {
        console.log("message received: ", event.data)
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
}

const send_message = (payload = "hello") => {
    if (check_connection()) {
        websocket.send(payload)
        console.log("client sent message: ", payload)
    } else {
        console.error(`websocket is not open, message "${payload}" not sent`)
    }
}

document.getElementById("join-room").addEventListener("click", () => {
    const join_button = document.getElementById("join-room")
    if (check_connection()) {
        join_button.textContent = "join room"
        close_connection()
    } else {
        join_button.textContent = "leave room"
        open_connection()
    }
})

