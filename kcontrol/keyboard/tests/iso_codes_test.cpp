/*
 *  Copyright (C) 2011 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <kdebug.h>
#include <QtGui/QApplication>
#include <qtest_kde.h>

#include "../iso_codes.h"


class IsoCodesTest : public QObject
{
    Q_OBJECT

    IsoCodes* isoCodes;

private Q_SLOTS:
    void initTestCase() {
    	isoCodes = new IsoCodes(IsoCodes::iso_639);
    }

    void cleanupTestCase() {
    	delete isoCodes;
    }

    void testIsoCodes() {
        QVERIFY( isoCodes != NULL );
        QVERIFY( ! isoCodes->getEntryList().isEmpty() );
        const IsoCodeEntry* isoEntry = isoCodes->getEntry(IsoCodes::attr_iso_639_2T_code, "eng");
        QVERIFY( isoEntry != NULL );
        QVERIFY( ! isoEntry->empty() );
        QCOMPARE( isoEntry->value(IsoCodes::attr_iso_639_2T_code), QString("eng") );
        QCOMPARE( isoEntry->value(IsoCodes::attr_iso_639_2B_code), QString("eng") );
        QCOMPARE( isoEntry->value(IsoCodes::attr_iso_639_1_code), QString("en") );
    }

    void loadIsoCodesBenchmark() {
    	QBENCHMARK {
    		IsoCodes* isoCodes = new IsoCodes(IsoCodes::iso_639);
    		delete isoCodes;
    	}
    }

};

//TODO: something lighter than KDEMAIN ?
QTEST_KDEMAIN( IsoCodesTest, NoGUI )

#include "iso_codes_test.moc"
