/*
 * Copyright 2008  Alex Merry <alex.merry@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "player.h"

Player::Player(PlayerFactory* factory)
    : m_factory(factory)
{
}

Player::~Player()
{
}

PlayerFactory* Player::factory() const
{
    return m_factory;
}

QString Player::name() const
{
    Q_ASSERT(!m_name.isEmpty());
    return m_name;
}

QString Player::artist()
{
    return QString();
}

QString Player::album()
{
    return QString();
}

QString Player::title()
{
    return QString();
}

int Player::trackNumber()
{
    return 0;
}

QString Player::comment()
{
    return QString();
}

QString Player::genre()
{
    return QString();
}

QString Player::lyrics()
{
    return QString();
}

int Player::length()
{
    return 0;
}

int Player::position()
{
    return 0;
}

float Player::volume()
{
    return -1;
}

QPixmap Player::artwork()
{
    return QPixmap();
}

bool Player::canPlay()
{
    return false;
}

void Player::play()
{
}

bool Player::canPause()
{
    return false;
}

void Player::pause()
{
}

bool Player::canStop()
{
    return false;
}

void Player::stop()
{
}

bool Player::canGoPrevious()
{
    return false;
}

void Player::previous()
{
}

bool Player::canGoNext()
{
    return false;
}

void Player::next()
{
}

bool Player::canSetVolume()
{
    return false;
}

void Player::setVolume(qreal)
{
}

bool Player::canSeek()
{
    return false;
}

void Player::seek(int)
{
}

void Player::setName(const QString& name)
{
    m_name = name;
}

