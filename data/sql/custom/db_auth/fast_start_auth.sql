CREATE TABLE IF NOT EXISTS `custom_account_fast_start` (
    `account_id` INT UNSIGNED NOT NULL,
    `account_name` VARCHAR(32) NOT NULL,
    `claims_available` INT UNSIGNED NOT NULL DEFAULT 0,
    `claims_used` INT UNSIGNED NOT NULL DEFAULT 0,
    `granted_by` VARCHAR(32) NOT NULL DEFAULT 'admin',
    `granted_at` DATETIME NOT NULL,
    PRIMARY KEY (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
