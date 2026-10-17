CREATE TABLE IF NOT EXISTS `custom_fast_start_log` (
    `id` INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    `account_id` INT UNSIGNED NOT NULL,
    `character_guid` INT UNSIGNED NOT NULL,
    `character_name` VARCHAR(50) NOT NULL,
    `class` TINYINT UNSIGNED NOT NULL,
    `role_chosen` VARCHAR(50) NOT NULL,
    `claimed_at` DATETIME NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
