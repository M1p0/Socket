var canvas = document.getElementById("bubble");
var signin = document.getElementsByClassName("text-right")[0];
var loginWrap = document.getElementsByClassName("login-wrap")[0];
var signinWrap = document.getElementsByClassName("signin-wrap")[0];
var loginRes = document.getElementById("login-res");
var signRes = document.getElementById("sign-res");
var loginUser = document.getElementById("login-user");
var loginPw = document.getElementById("login-pw");
var loginBtn = document.getElementById("login-btn");
var signinUser = document.getElementById("signin-user");
var signinPw = document.getElementById("signin-pw");
var signinBtn = document.getElementById("signin-btn");

var url = "http://192.168.1.2:9001/api";
var aColor = ["#ddf0ed", "#f2efe6", "#c7ffec", "#f17c67", "#00ff80", "#25c6fc", "3b200c"];
var aBubble = [];
var w, h; //浏览器宽高
var canCon = canvas.getContext("2d");


(function setSize() { //使canvas动态适应浏览器宽高
    window.onresize = arguments.callee; //函数自调用 监听浏览器宽高变化
    w = window.innerWidth;
    h = window.innerHeight;
    canvas.width = w;
    canvas.height = h;
})();

function random(min, max) { //重新封装random , 随机一个min与max之间的数
    return Math.random() * (max - min) + min;
}

function Bubble() {}; //构造球对象
Bubble.prototype = {
    init: function () {
        this.x = random(0, w);
        this.y = random(0, h);
        this.r = random(1, 4);
        this.color = aColor[Math.floor(random(0, 7))];
        this.vx = random(-2, 2); //x方向的速度
        this.vy = random(-2, 2); //y方向的速度
    },
    draw: function () {
        canCon.beginPath();
        canCon.fillStyle = this.color;
        canCon.arc(this.x, this.y, this.r, 0, Math.PI * 2);
        canCon.fill();
    },
    move: function () {
        this.x += this.vx;
        this.y += this.vy;
        if (this.x - this.r < 0 || this.x + this.r > w) {
            this.vx = -this.vx;
        }
        if (this.y - this.r < 0 || this.y + this.r > h) {
            this.vy = -this.vy;
        }
        this.draw();
    }
}

function create(num) {
    for (var i = 0; i < num; i++) {
        var bubble = new Bubble();
        bubble.init();
        bubble.draw();
        aBubble.push(bubble);
    }
}
create(2000);

setInterval(function () {
    canCon.clearRect(0, 0, w, h);
    for (var item of aBubble) {
        item.move();
    }
}, 0.00001);


signin.addEventListener("click",function(){
    loginWrap.style.display = "none";
    signinWrap.style.display = "block";
});

loginBtn.addEventListener("click",function(){
    var user = loginUser.value;
    var pw = loginPw.value;
    var s = '{"command":"login","id":"'+user+'","password":"'+pw+'"}';
    var len = s.length;
    var xml = new XMLHttpRequest();
    xml.open("POST",url);
    xml.setRequestHeader("Content-type","application/x-www-form-urlencoded");
    //xml.setRequestHeader("Content-length", len); 
    xml.send(s);
    xml.onreadystatechange = function(){
        if(xml.readyState === 4 && xml.status ===200){
            var res = JSON.parse(xml.responseText);
            console.log(res)
            if(res.status === "success"){
                window.location.href= 'MyIM.html?user='+user+'&pw='+pw+'&username='+res.username;
            }else{
                loginRes.style.display = "block";
            }
        }
    }
})

signinBtn.addEventListener("click",function(){
    var user = signinUser.value;
    var pw = signinPw.value;
    var s = '{"command":"logon","username":"'+user+'","password":"'+pw+'"}';
    var _href = 'MyIm.html?user='+user+'&pw='+pw;
    var len = s.length;
    var xml = new XMLHttpRequest();
    xml.open("POST",url);
    xml.setRequestHeader("Content-type","application/x-www-form-urlencoded");
    //xml.setRequestHeader("Content-length", len); 
    xml.send(s);
    xml.onreadystatechange = function(){
        if(xml.readyState === 4 && xml.status ===200){
            var res = JSON.parse(xml.responseText);
            if(res.status === "success"){
                alert("注册成功,你的id为:"+res.id);
                signinWrap.style.display = "none";
                loginWrap.style.display = "block";
            }else{
                signRes.style.display = "block";
            }
        }
    }
})
