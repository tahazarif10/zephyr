/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/ztest.h>

static bt_addr_le_t oob_test_addr(uint8_t value)
{
	bt_addr_le_t addr = {
		.type = BT_ADDR_LE_RANDOM,
		.a.val = {value, 0x22, 0x33, 0x44, 0x55, 0xC0},
	};

	return addr;
}

static size_t oob_identity_count(void)
{
	size_t count = 0U;

	bt_id_get(NULL, &count);
	return count;
}

ZTEST(bt_id_public_api, test_oob_rejects_invalid_identity)
{
	struct bt_le_oob oob = {0};
	bt_addr_le_t addr1 = oob_test_addr(1U);
	bt_addr_le_t addr2 = oob_test_addr(2U);
	uint8_t first_unused_id = (uint8_t)oob_identity_count();
	int id1;
	int id2;
	int err;

	err = bt_le_oob_get_local(first_unused_id, &oob);
	zassert_equal(err, -EINVAL, "Unused identity returned %d", err);

	id1 = bt_id_create(&addr1, NULL);
	id2 = bt_id_create(&addr2, NULL);
	zassert_equal(id1, 1, "Unexpected first secondary identity id");
	zassert_equal(id2, 2, "Unexpected second secondary identity id");

	zassert_ok(bt_id_delete((uint8_t)id1), "Failed to delete non-tail identity");

	err = bt_le_oob_get_local((uint8_t)id1, &oob);
	zassert_equal(err, -EINVAL, "Deleted identity returned %d", err);

	zassert_ok(bt_le_oob_get_local((uint8_t)id2, &oob), "Valid identity OOB lookup failed");
	zassert_mem_equal(&oob.addr, &addr2, sizeof(addr2), "Valid identity address was not returned");
}
