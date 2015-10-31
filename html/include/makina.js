function almahide(a) {
    if (a) {
        a.each(function(elem) {elem.hide();});
    }
}

function almashow(a) {
    if (a) {
        a.each(function(elem) {elem.show();});
    }
}

function almashowa(a, idx) {
    if(a) {
        a.each(function(elem) {elem.ancestors()[idx].show();});
    }
}
function almatheme() {
    if($$('.menuitem a').length<3) {
        almahide($$('#menuitem_posixAccount'));
        almahide($$('#menuitem_icon_posixAccount'));
        almahide($$('fieldset#perso'));
        almahide($$('fieldset#contact'));
        almahide($$('fieldset#homecontact'));
        almahide($$('fieldset#organization'));
        almahide($$('fieldset#account tr'));
        almashowa($$('fieldset#account #userPassword_password'), 1);
        almashowa($$('fieldset#account #userPassword_password2'), 1);
    }
}
document.observe("dom:loaded", almatheme);
