-- Task 10: First Character Gets 10 Gold (once per account)
CREATE TABLE IF NOT EXISTS `account_first_char_bonus` (
    `account_id` INT UNSIGNED NOT NULL PRIMARY KEY,
    `char_guid` BIGINT UNSIGNED NOT NULL,
    `rewarded_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Prevent retroactive bonus for existing accounts that already have characters
INSERT IGNORE INTO `account_first_char_bonus` (`account_id`, `char_guid`)
SELECT `account`, MIN(`guid`) FROM `characters` GROUP BY `account`;
