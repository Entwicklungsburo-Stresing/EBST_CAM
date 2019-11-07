COMSCORE.SiteRecruit.Broker.config={sv:"scor",cddsDomains:"www.windowsphone.com|www.microsoftstore.com|office.com|xbox.com",cddsInProgress:"cddsinprogress",domainSwitch:"tracking3p",domainMatch:"([\\da-z.-]+.com)",delay:0,cddsIntervalMax:10,crossDomainCheck:function(){if(this.cddsIntervalMax>1){this.cddsIntervalMax--;
if(COMSCORE.SiteRecruit.Utils.UserPersistence.getCookieValue(this.cddsInProgress)!=false){setInterval(function() { COMSCORE.SiteRecruit.DDKeepAlive.setDDTrackerCookie()}, 1000);
COMSCORE.SiteRecruit._halt=true;this.clearCrossDomainCheck();}}else{this.clearCrossDomainCheck();
}},clearCrossDomainCheck:function(){window.clearInterval(crossDomainInterval);},isolateDomain:function(A){A=A.substring(A.indexOf("//")+2,A.length);
A=A.substring(0,A.indexOf("/"));return A;},testMode:false,addEventDelay:1000,cookie:{name:"msresearch",path:"/",domain:".microsoft.com",duration:90,rapidDuration:0,expireDate:""},tracker:{std:"http://www.microsoft.com/library/svy/sto/https/SiteRecruit_Tracker.htm",ssl:"https://www.microsoft.com/library/svy/sto/https/SiteRecruit_Tracker.htm"},mobile:{match:"iphone|ipad|ipod|android|opera mini|blackberry|windows (phone|ce)|iemobile|htc|nokia|bb10|mobile safari|mobile|wpdesktop|lumia|playbook|tablet",kmatch:"(?:; ([^;)]+) Build/.*)?\bSilk/([0-9._-]+)\b(.*\bMobile Safari\b)?",halt:true},graceIncr:{name:"graceIncr",initDelay:0,clickDelay:5000,match:"https://(account|accounts|billing|commerce|support|login|live).(microsoft|live|xbox).(com)",altTag:"class",htmlMatch:"sign in"},prefixUrl:"",mapping:[
//{m:"technet.microsoft.com/en-us",c:"inv_c_p246609455-technet.js",f:0.0,p:0,halt:true}
//{m:"www.microsoft.com/empowering-countries/en-us/",c:"inv_c_p346498099-3132.js",f:0.0,p:0,halt:true}
{m:"enterprise.microsoft.com/en-au",c:"inv_c_p346498099-en-au.js",f:0.5,p:0},
{m:"enterprise.microsoft.com/en-au",c:"inv_c_p346498099-en-au-M.js",f:0.5,p:2,prereqs:{content:[],cookie:[{name:"srMOB",value:"1"}]}},
{m:"enterprise.microsoft.com/en-ca",c:"inv_c_p346498099-en-ca.js",f:0.5,p:0},
{m:"enterprise.microsoft.com/en-ca",c:"inv_c_p346498099-en-ca-M.js",f:0.5,p:2,prereqs:{content:[],cookie:[{name:"srMOB",value:"1"}]}},
{m:"enterprise.microsoft.com/en-us",c:"inv_c_p346498099-en-us.js",f:0.5,p:0}
,{m:"enterprise.microsoft.com/en-us", c: "inv_c_p346498099-en-us-M.js", f: 0.5, p: 2 , prereqs:{content:[],cookie:[{'name':'srMOB','value':'1'}]}}

,{m:"enterprise.microsoft.com/en-gb",c:"inv_c_p346498099-en-gb.js",f:0.5,p:0}
,{m: "enterprise.microsoft.com/en-gb", c: "inv_c_p346498099-en-gb-M.js", f: 0.5, p: 2 , prereqs:{content:[],cookie:[{'name':'srMOB','value':'1'}]}}
/*,{m:"enterprise.microsoft.com/fr-fr",c:"inv_c_p346498099-fr-fr.js",f:0.0,p:0,halt:true}
,{m: "enterprise.microsoft.com/fr-fr", c: "inv_c_p346498099-fr-fr-M.js", f: 0.0, p: 2, halt:true, prereqs:{content:[],cookie:[{'name':'srMOB','value':'1'}]}}
,{m:"enterprise.microsoft.com/ja-jp",c:"inv_c_p346498099-ja-jp.js",f:0.0,p:0,halt:true}
,{m: "enterprise.microsoft.com/ja-jp", c: "inv_c_p346498099-ja-jp-M.js", f: 0.0, p: 2, halt:true, prereqs:{content:[],cookie:[{'name':'srMOB','value':'1'}]}}
*/
,{m:"enterprise.microsoft.com/de-de",c:"inv_c_p346498099-de-de.js",f:0.5,p:0}
,{m: "enterprise.microsoft.com/de-de", c: "inv_c_p346498099-de-de-M.js", f: 0.5, p: 2 , prereqs:{content:[],cookie:[{'name':'srMOB','value':'1'}]}},
{m:"enterprise.microsoft.com/fr-ca",c:"inv_c_p346498099-fr-ca.js",f:0.5,p:0},
{m:"enterprise.microsoft.com/fr-ca",c:"inv_c_p346498099-fr-ca-M.js",f:0.5,p:2,prereqs:{content:[],cookie:[{name:"srMOB",value:"1"}]}},

,{m: 'channelincentivesbeta2.microsoft.com/CHIPUI', c: 'inv_c_p346231629-en-us.js', f: 100, p: 0 ,prereqs:{content:[{'element':'a','elementValue':'Privacy (&amp;|&) Cookies','attrib':'','attribValue':''}],cookie:[]}}
],Events:{beforeRecruit:function(){
var C=COMSCORE.SiteRecruit.Broker.config;
var B=COMSCORE.SiteRecruit.Utils.UserPersistence;
/*if(/(technet|msdn)\.microsoft\.com\/en-us/i.test(window.location)) {
  if(/windowsmediaplayer|(microsoft|microsoftbusinesshub|xbox|windowsphone|windows|skype|live|microsoftvirtualacademy|microsoftvolumelicensing|microsoftstore|office|onenote|visualstudio|outlook)\.com/i.test(document.referrer)){
    COMSCORE.SiteRecruit._halt = true;
  }
}*/
//Mobile
if (/enterprise\.microsoft\.com/i.test(window.location)) {
  if(COMSCORE.SiteRecruit.device.type>1){
    B.createCookie("srMOB", "1", {path:'/',domain:C.cookie.domain,duration:'s'})
    COMSCORE.SiteRecruit.Broker.config.mobile.halt = false;
 }}
if(B.getCookieValue(C.domainSwitch)!=false){B.createCookie(C.domainSwitch,"",{path:"/",domain:C.cookie.domain,duration:-1});
}COMSCORE.SiteRecruit.Broker.custom={captLinks:function(F){var E=B.getCookieValue("captLinks");
var D="";if(E==false){D=escape(F)+";";}else{if(D.length+E.length<1440){D=E+escape(F)+";";
}}if(D!=""){B.createCookie("captLinks",D,{path:"/",domain:C.cookie.domain,duration:"s"});
}},allTags:function(H,J,I,G){if(H=="class"){if(/msie (8|7)/i.test(navigator.userAgent)){return;
}var D=document.getElementsByClassName(J);}else{var D=document.getElementsByTagName(H);
}var E=new RegExp(I,"i");for(var F=0;F<D.length;F++){if((H=="a"&&E.test(D[F].href))||(H=="class"&&E.test(D[F].innerHTML))){if(D[F].addEventListener){if(G==1){D[F].addEventListener("click",function(K){if(E.test(this.href)){B.createCookie(C.domainSwitch,C.isolateDomain(this.href),{path:"/",domain:C.cookie.domain,duration:"s"});
}},false);}else{if(G==2){D[F].addEventListener("click",function(K){B.createCookie("graceIncr",1,{path:"/",domain:C.cookie.domain,duration:"s"});
},false);}else{if(G==3&&COMSCORE.isDDInProgress()){D[F].addEventListener("click",function(K){COMSCORE.SiteRecruit.Broker.custom.captLinks(this.href);
},false);}}}}else{hrefURL=D[F].href;if(G==1){D[F].attachEvent("onclick",function(){B.createCookie(C.domainSwitch,C.isolateDomain(hrefURL),{path:"/",domain:C.cookie.domain,duration:"s"});
});}else{if(G==2){D[F].attachEvent("onclick",function(){B.createCookie("graceIncr",1,{path:"/",domain:C.cookie.domain,duration:"s"});
});}else{if(G==3&&COMSCORE.isDDInProgress()){D[F].attachEvent("onclick",function(){COMSCORE.SiteRecruit.Broker.custom.captLinks(this.href);
});}}}}}}}};var A=0;if(COMSCORE.SiteRecruit.Utils.UserPersistence.getCookieValue("graceIncr")==1){A=5000;
}setTimeout(function(){COMSCORE.SiteRecruit.Utils.UserPersistence.createCookie("graceIncr",0,{path:"/",domain:C.cookie.domain,duration:"s"});
},A);setTimeout(function(){COMSCORE.SiteRecruit.Broker.custom.allTags("a","",C.cddsDomains,1);
},C.addEventDelay);setTimeout(function(){COMSCORE.SiteRecruit.Broker.custom.allTags("a","",C.graceIncr.match,2);
},C.addEventDelay);}}};var crossDomainInterval=window.setInterval("COMSCORE.SiteRecruit.Broker.config.crossDomainCheck()","1000");
if(COMSCORE.SiteRecruit.Broker.delayConfig==true){COMSCORE.SiteRecruit.Broker.config.delay=5000;
}window.setTimeout("COMSCORE.SiteRecruit.Broker.run()",COMSCORE.SiteRecruit.Broker.config.delay);