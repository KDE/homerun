/*
 *   Copyright 2012 Aurélien Gâteau <agateau@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
var pages = new Array();
var currentIndex = -1;

function addPage(page) {
    if (currentIndex > -1) {
        // Remove pages after currentIndex
        var lst = pages.splice(currentIndex + 1);
        lst.forEach(function(x) { x.destroy(); });
    }
    pages.push(page);
}

function goTo(index) {
    if (currentIndex != -1) {
        pages[currentIndex].opacity = 0;
    }
    currentIndex = index;
    pages[index].opacity = 1;
    currentPage = pages[index];
    _updateCanGoBackForward();
}

function goBack() {
    if (currentIndex > 0) {
        goTo(currentIndex - 1);
    }
}

function goForward() {
    if (currentIndex < pages.length - 1) {
        goTo(currentIndex + 1);
    }
}

function _updateCanGoBackForward() {
    canGoBack = currentIndex > 0;
    canGoForward = currentIndex < pages.length - 1;
}
