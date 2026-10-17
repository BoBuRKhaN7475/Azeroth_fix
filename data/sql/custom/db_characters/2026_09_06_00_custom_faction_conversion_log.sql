-- Task 19: Cross-Faction Item Conversion Character Audit Log
CREATE TABLE IF NOT EXISTS `custom_faction_item_conversion_log` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `character_guid` BIGINT UNSIGNED NOT NULL,
  `account_id` INT UNSIGNED NOT NULL,
  `character_name` VARCHAR(50) NOT NULL,
  `source_item_id` INT UNSIGNED NOT NULL,
  `target_item_id` INT UNSIGNED NOT NULL,
  `gem1` INT UNSIGNED NOT NULL DEFAULT 0,
  `gem2` INT UNSIGNED NOT NULL DEFAULT 0,
  `gem3` INT UNSIGNED NOT NULL DEFAULT 0,
  `enchant_id` INT UNSIGNED NOT NULL DEFAULT 0,
  `durability` INT UNSIGNED NOT NULL DEFAULT 0,
  `converted_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_char_guid` (`character_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
