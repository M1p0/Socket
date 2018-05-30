(function(window){
    /**
     * 选项卡组件类
     * @param {element} tab tab组件
     */
    function Tab(tab){
        
        var that = this;
        
        //保存tab组件
        this.tab = tab;
        //默认配置
        this.config = {
            "triggerType":"mouseover",//触发事件:click或mouseover
            "invoke":1,//初始标签页
            "auto":false // 是否可以自动切换
        }
        //保存标签列表和对应内容
        this.tabNav = this.tab.getElementsByClassName("tab-nav")[0];
        this.tabContentItem = this.tab.getElementsByClassName("content-item");

        var brother = this.tabNav.children;
        var len = brother.length;

        //更新配置
        if(this.getConfig()){
            if(this.getConfig().triggerType){
                this.config.triggerType = this.getConfig().triggerType;
            }
            if(this.getConfig().invoke){
                this.config.invoke = this.getConfig().invoke;
            }
            if(this.getConfig().auto){
                this.config.auto = this.getConfig().auto;
            }
        }
        
        var config = this.config;
        //通过事件委托绑定事件
        if(config.triggerType === "click"){
            this.tabNav.addEventListener("click",function(e){  
                var e = e || window.event;
                var target = e.target || e.srcElement;
                if(target.tagName === "A" || target.tagName === "LI"){
                    that.invoke(target)
                }
            })
        }else{
            this.tabNav.addEventListener("mouseover",function(e){
                var e = e || window.event;
                var target = e.target || e.srcElement;
                if(target.tagName === "A" || target.tagName === "LI"){
                    that.invoke(target)
                }
            })
        }
        if(this.config.auto && typeof this.config.auto === "number"){
            this.timer = null;
            this.counter = 0;

            this.autoPlay();

            this.tab.addEventListener("mouseover",function(){
                clearInterval(that.timer);
            });
            this.tab.addEventListener("mouseout",function(){
                that.autoPlay();
            });
        }

        if(this.config.invoke!==1){
            this.invoke(brother[(this.config.invoke-1)%len]);
        }
    }

    Tab.prototype = {
        //获取配置参数
        getConfig:function(){
            var config = this.tab.getAttribute('data-config');
            if(config && config !== ""){
                return JSON.parse(config);
            }else{
                return null;
            }
        },
        //事件驱动函数
        invoke:function(curNode){
            if(curNode.tagName !== "LI"){
                curNode = curNode.parentNode;
            }
            var brother = curNode.parentNode.children;
            var effect = this.config.effect;
            var conItems = this.tabContentItem;

            //移除actived类名,current类名
            for(var i=0,len=brother.length;i<len;i++){
                brother[i].index = i;
                brother[i].classList.remove("actived");
                conItems[i].classList.remove("current");
            }
            var index = curNode.index;
            //给当前节点添加类名,切换内容页
            curNode.classList.add("actived");
            conItems[index].classList.add("current");

            if(this.config.auto && typeof this.config.auto === "number"){
                this.counter = index;
            }
    
        },
        autoPlay:function(){
            var that = this, brother = this.tabNav.children, conItems = this.tabContentItem, len = brother.length, config = this.config;

            clearInterval(this.timer);

            this.timer = setInterval(function(){
                that.counter = (that.counter+1)%len;
                
                for(var i=0;i<len;i++){
                    brother[i].index = i;
                    brother[i].classList.remove("actived");
                    conItems[i].classList.remove("current");
                }
                //给当前节点添加类名,切换内容页
                brother[that.counter].classList.add("actived");
                conItems[that.counter].classList.add("current");

            },config.auto);
        }
    }

    Tab.init = function(tabs){
        
        for(var i=0,len=tabs.length;i<len;i++){
            new this(tabs[i])
        }
    }

    window.Tab = Tab;
})(window)