/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.2
import MuseScore 3.0

MuseScore {

    id: root

    title: "Test WebSocket"

    property int connectedToServerSocketId: -1
    property int clientSocketId: -1

    function onMessageToServer(msg) {
        console.log("received message to server: " + msg)
    }

    function onMessageToClient(msg) {
        console.log("received message to client: " + msg)
    }

    function onConnected() {
        // NOTE Send message from client to server
        api.websocket.send(root.clientSocketId, "msg from client")

        // NOTE Send message from server to client
        api.websocketserver.send(root.connectedToServerSocketId, "msg from server")
    }

    onRun: {
        console.log("Run " + root.title);

        //! NOTE Listen server
        api.websocketserver.listen(8084, function(id) {
            console.log("new connected to server, client id: " + id);
            if (root.connectedToServerSocketId === -1) {
                root.connectedToServerSocketId = id;
                api.websocketserver.onMessage(root.connectedToServerSocketId, root.onMessageToServer)
            }
        })

        //! NOTE Open client
        api.websocket.open(8084, function(id) {
            console.log("connected socket id: " + id);
            root.clientSocketId = id;

            api.websocket.onMessage(root.clientSocketId, root.onMessageToClient)

            root.onConnected()
        });
    }
}
