setTimeout(function(){
    var el = document.createElement('p');
    el.innerHTML = 'This DOM was created by javascript';
    document.getElementsByTagName('body')[0].appendChild(el);
}, 500);
