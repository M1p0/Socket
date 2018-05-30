var httpUrl = "http://127.0.0.1:9001/api";
var webSocketUrl = "ws://127.0.0.1:9002";
var id = getQueryStringByName("user");
var username = getQueryStringByName("username");
var pw = getQueryStringByName("pw");
var oUser = document.getElementById("user");
var friend = document.getElementById("friend");
var tabNav = document.getElementsByClassName("tab-nav")[0];
var tabItem = document.getElementsByClassName("tab-item");
var oContainer = document.getElementById("container");
var chatUL = document.getElementById("chart");
var websocket = new WebSocket(webSocketUrl);
var msg = {};

window.addEventListener("DOMContentLoaded", init);

function init() {
    tabNav.addEventListener("click", function (e) {
        var e = e || window.event;
        var target = e.target || e.srcElement;
        if (target.tagName === "A") {
            tagChange(target);
        }
    });
    oUser.innerHTML = username;

    getFriendList();
    createWS();
};


(function setSize() {
    window.onresize = arguments.callee;
    var h = window.innerHeight;
    document.body.style.height = h + "px";
})();


function getQueryStringByName(name) {
    var result = location.search.match(new RegExp("[\?\&]" + name + "=([^\&]+)", "i"));
    if (result == null || result.length < 1) {
        return "";
    }
    return result[1];
}

function tagChange(curNode) {
    curNode = curNode.parentNode;
    var brother = curNode.parentNode.children;
    //移除actived类名,current类名
    for (var i = 0, len = brother.length; i < len; i++) {
        brother[i].index = i;
        brother[i].classList.remove("select");
        tabItem[i].classList.remove("current");
    }
    var index = curNode.index;
    curNode.classList.add("select");
    tabItem[index].classList.add("current");
}

function getFriendList() {
    var friendContent = friend.innerHTML;
    var s = '{"command":"list_friend","id":"' + id + '"}';
    var xml = new XMLHttpRequest();
    xml.open("POST", httpUrl);
    xml.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    xml.send(s);
    xml.onreadystatechange = function () {
        if (xml.readyState === 4 && xml.status === 200) {
            var res = JSON.parse(xml.responseText);
            if (res.status === "success") {
                var friendList = res.friends;
                for (let i = 0, len = friendList.length; i < len; i++) {
                    let item = friendList[i];
                    friendContent += '<li id="' + item.id + '" class="friend-list-item list-item list"><div class="avatar"><img></div><p class="nick">' + item.id + '</p><p class="shuoshuo">...</p></li>';
                };
                friend.innerHTML = friendContent;
                friend.style.display = "block";
                friendAddEvent();
            } else {
                console.log(res.detail);
            }
        }
    }
}

function friendAddEvent() {
    let aFriendLI = document.getElementsByClassName("friend-list-item");

    for (let i = 0, len = aFriendLI.length; i < len; i++) {
        aFriendLI[i].addEventListener("click", function () {
            var thisId = this.id;
            var aChatPanel = document.getElementsByClassName("chat-panel");
            if (aChatPanel.length !== 0) {
                for (var j = 0, l = aChatPanel.length; j < l; j++) {
                    aChatPanel[j].style.display = "none";
                }
            }
            var chatPanelId = document.getElementById("chatPanel" + this.id);
            if (chatPanelId) {
                chatPanelId.style.display = "block";
                addPanelEvent(this.id);
            } else {
                oContainer.innerHTML += '<div class="chat-panel" id="chatPanel' + this.id + '"><div class="top"><div class="head-content"><header class="chat-header"><h1>' + this.id + '</h1><button id="closebtn' + this.id + '" class="closebtn">关闭</button></header><div id="chat-content' + this.id + '" class="chat-content"></div></div></div><footer class="chat-footer"><input id="input' + this.id + '" class="_input" type="text" maxlength="10"><button id="sendbtn' + this.id + '" class="sendbtn">发送</button></footer></div>';

                addPanelEvent(this.id);

            }
        });
    }
}

function addPanelEvent(thisId) {
    var clsbtn = document.getElementById("closebtn" + thisId);
    clsbtn.onclick = function () {
        var chatPanelId = document.getElementById("chatPanel" + thisId);
        chatPanelId.style.display = "none";
    };

    var sendbtn = document.getElementById("sendbtn" + thisId);
    var input = document.getElementById("input" + thisId);
    var chatContent = document.getElementById("chat-content" + thisId);

    sendbtn.onclick = function () {
        var chatLi = document.getElementById("chat-list" + thisId);
        var txt = input.value;
        var s = '{"command":"send_message","src":"' + id + '","dst":"' + thisId + '","message":"' + txt + '"}';
        input.value = "";
        chatContent.innerHTML += '<div class="content"><div class="avatar"><img></div><p class="chat_nick">' + username + '</p><p class="chat_content ">' + txt + '</p></div>';
        if (chatLi) {
            var lastmsg = document.getElementById("lastmsg" + thisId);
            lastmsg.innerHTML = txt;
        } else {
            chatUL.innerHTML += '<li id="chat-list' + thisId + '" class="chat-list-item list-item list"><div class="avatar"><img></div><p class="nick">' + thisId + '</p><p id="lastmsg' + thisId + '" class="msg">' + txt + '</p></li>';
            var chatLi = document.getElementsByClassName("chat-list-item");
            for(var i=0,len=chatLi.length;i<len;i++){
                addChatListEvent(chatLi[i].id)
            }
        }
        websocket.send(s);
    }
}

function addChatListEvent(chatLiId){
    var chatLi = document.getElementById(chatLiId);
    chatLi.onclick = function(){
        
        var thisId = this.id.split("chat-list")[1];
        var chatPanelId = document.getElementById("chatPanel" + thisId);
        if(chatPanelId){
            var aChatPanel = document.getElementsByClassName("chat-panel");
            if (aChatPanel.length !== 0) {
                for (var j = 0, l = aChatPanel.length; j < l; j++) {
                    aChatPanel[j].style.display = "none";
                }
            }
            chatPanelId.style.display = "block";
            addPanelEvent(thisId);
        }else{
            oContainer.innerHTML += '<div class="chat-panel" id="chatPanel' + thisId + '"><div class="top"><div class="head-content"><header class="chat-header"><h1>' + thisId + '</h1><button id="closebtn' + thisId + '" class="closebtn">关闭</button></header><div id="chat-content' + thisId + '" class="chat-content"></div></div></div><footer class="chat-footer"><input id="input' + thisId + '" class="_input" type="text" maxlength="10"><button id="sendbtn' + thisId + '" class="sendbtn">发送</button></footer></div>';

            addPanelEvent(thisId);
        }
    }

    
}

function createWS() {
    websocket.onopen = function () { //连接事件
        var s = '{"command":"login","id":"' + id + '","password":"' + pw + '"}';
        console.log("建立websocket连接");
        websocket.send(s);
    }
    websocket.onclose = function () { //关闭连接事件
        console.log("关闭websocket连接");
    }
    websocket.onmessage = function (e) { //接受信息的事件

        var data = JSON.parse(e.data);
        if(data.command === "send_message"){
            var chatLi = document.getElementById("chat-list"+data.src);
            if(msg[data.src]){
                msg[data.src].push(data.message);
            }else{
                msg[data.src] = [];
                msg[data.src].push(data.message);
            }
            if (chatLi) {
                var lastmsg = document.getElementById("lastmsg" + data.src);
                lastmsg.innerHTML = data.message;
                var chatContent = document.getElementById('chat-content' + data.src);
                if(chatContent){
                    chatContent.innerHTML += '<div class="content"><div class="avatar"><img></div><p class="chat_nick">' + data.src + '</p><p class="chat_content ">' + data.message + '</p></div>';
                }
            } else {
                var chatContent = document.getElementById('chat-content' + data.src);
                if(chatContent){
                    chatContent.innerHTML += '<div class="content"><div class="avatar"><img></div><p class="chat_nick">' + data.src + '</p><p class="chat_content ">' + data.message + '</p></div>';
                    chatUL.innerHTML += '<li id="chat-list' + data.src + '" class="chat-list-item list-item list"><div class="avatar"><img></div><p class="nick">' + data.src + '</p><p id="lastmsg' + data.src + '" class="msg">' + data.message + '</p></li>';
                    var chatLi = document.getElementsByClassName("chat-list-item");
                    for(var i=0,len=chatLi.length;i<len;i++){
                        addChatListEvent(chatLi[i].id)
                    }
                }else{
                    chatUL.innerHTML += '<li id="chat-list' + data.src + '" class="chat-list-item list-item list"><div class="avatar"><img></div><p class="nick">' + data.src + '</p><p id="lastmsg' + data.src + '" class="msg">' + data.message + '</p></li>';
                    var chatLi = document.getElementsByClassName("chat-list-item");
                    for(var i=0,len=chatLi.length;i<len;i++){
                        chatLi[i].onclick = function(){
                            
                            var aChatPanel = document.getElementsByClassName("chat-panel");
                            if (aChatPanel.length !== 0) {
                                for (var j = 0, l = aChatPanel.length; j < l; j++) {
                                    aChatPanel[j].style.display = "none";
                                }
                            }
                            var thisId = this.id.split("chat-list")[1];
                            var chatPanelId = document.getElementById("chatPanel" + thisId);
                            if(chatPanelId){  
                                chatPanelId.style.display = "block";
                                addPanelEvent(thisId);
                            }else{

                                var chatContent = document.getElementById('chat-content' + data.src);
                                if(chatContent){
                                    chatContent.innerHTML += '<div class="content"><div class="avatar"><img></div><p class="chat_nick">' + data.src + '</p><p class="chat_content ">' + data.message + '</p></div>';
                                }else{
                                    var content = "";
                                    content += '<div class="chat-panel" id="chatPanel' + thisId + '"><div class="top"><div class="head-content"><header class="chat-header"><h1>' + thisId + '</h1><button id="closebtn' + thisId + '" class="closebtn">关闭</button></header><div id="chat-content' + thisId + '" class="chat-content">';
                                    
                                    for(var m=0;m<msg[thisId].length;m++){
                                        content += '<div class="content"><div class="avatar"><img></div><p class="chat_nick">' + thisId + '</p><p class="chat_content ">' + msg[thisId][m] + '</p></div>';
                                    }
                                    
                                    content += '</div></div></div><footer class="chat-footer"><input id="input' + thisId + '" class="_input" type="text" maxlength="10"><button id="sendbtn' + thisId + '" class="sendbtn">发送</button></footer></div>';
                                    console.log(content)
                                    oContainer.innerHTML += content;
                                }
                                
                    
                                addPanelEvent(thisId);
                            }
                        }
                    }
                }
                
            }
            
        }
    }

    
}