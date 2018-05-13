-- MySQL Script generated by MySQL Workbench
-- Sun May 13 11:29:26 2018
-- Model: New Model    Version: 1.0
-- MySQL Workbench Forward Engineering

SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL,ALLOW_INVALID_DATES';

-- -----------------------------------------------------
-- Schema HR_Migration_System
-- -----------------------------------------------------

-- -----------------------------------------------------
-- Schema HR_Migration_System
-- -----------------------------------------------------
CREATE SCHEMA IF NOT EXISTS `HR_Migration_System` DEFAULT CHARACTER SET utf8 ;
USE `HR_Migration_System` ;

-- -----------------------------------------------------
-- Table `HR_Migration_System`.`Interviewee`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `HR_Migration_System`.`Interviewee` ;

CREATE TABLE IF NOT EXISTS `HR_Migration_System`.`Interviewee` (
  `interviewee_id` INT NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`interviewee_id`),
  UNIQUE INDEX `interviewee_id_UNIQUE` (`interviewee_id` ASC))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `HR_Migration_System`.`Interviewer`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `HR_Migration_System`.`Interviewer` ;

CREATE TABLE IF NOT EXISTS `HR_Migration_System`.`Interviewer` (
  `interviewer_id` INT NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`interviewer_id`),
  UNIQUE INDEX `interviewer_id_UNIQUE` (`interviewer_id` ASC))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `HR_Migration_System`.`HR_manager`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `HR_Migration_System`.`HR_manager` ;

CREATE TABLE IF NOT EXISTS `HR_Migration_System`.`HR_manager` (
  `hr_id` INT NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`hr_id`),
  UNIQUE INDEX `hr_id_UNIQUE` (`hr_id` ASC))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `HR_Migration_System`.`Position`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `HR_Migration_System`.`Position` ;

CREATE TABLE IF NOT EXISTS `HR_Migration_System`.`Position` (
  `position_id` INT NOT NULL,
  `job` VARCHAR(45) NULL,
  `job_title` VARCHAR(45) NULL,
  `expected_salary` VARCHAR(45) NULL,
  `location` VARCHAR(45) NULL,
  `hr_id` INT NULL,
  PRIMARY KEY (`position_id`),
  UNIQUE INDEX `position_id_UNIQUE` (`position_id` ASC),
  INDEX `hr_id_idx` (`hr_id` ASC),
  CONSTRAINT `hr_id`
    FOREIGN KEY (`hr_id`)
    REFERENCES `HR_Migration_System`.`HR_manager` (`hr_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `HR_Migration_System`.`Plays`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `HR_Migration_System`.`Plays` ;

CREATE TABLE IF NOT EXISTS `HR_Migration_System`.`Plays` (
  `ee_id` INT NULL,
  `username` VARCHAR(20) NOT NULL,
  `er_id` INT NULL,
  `hr_id` INT NULL,
  PRIMARY KEY (`username`),
  UNIQUE INDEX `username_UNIQUE` (`username` ASC),
  INDEX `ee_id_idx` (`ee_id` ASC),
  INDEX `er_id_idx` (`er_id` ASC),
  INDEX `hr_id_idx` (`hr_id` ASC),
  CONSTRAINT `ee_id`
    FOREIGN KEY (`ee_id`)
    REFERENCES `HR_Migration_System`.`Interviewee` (`interviewee_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `er_id`
    FOREIGN KEY (`er_id`)
    REFERENCES `HR_Migration_System`.`Interviewer` (`interviewer_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `hr_id`
    FOREIGN KEY (`hr_id`)
    REFERENCES `HR_Migration_System`.`HR_manager` (`hr_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `HR_Migration_System`.`User`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `HR_Migration_System`.`User` ;

CREATE TABLE IF NOT EXISTS `HR_Migration_System`.`User` (
  `username` VARCHAR(20) NOT NULL,
  `email` VARCHAR(20) NULL,
  `name` VARCHAR(20) NULL,
  `Plays_username` VARCHAR(20) NOT NULL,
  PRIMARY KEY (`username`),
  UNIQUE INDEX `username_UNIQUE` (`username` ASC),
  INDEX `fk_User_Plays_idx` (`Plays_username` ASC),
  CONSTRAINT `fk_User_Plays`
    FOREIGN KEY (`Plays_username`)
    REFERENCES `HR_Migration_System`.`Plays` (`username`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `HR_Migration_System`.`Applies`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `HR_Migration_System`.`Applies` ;

CREATE TABLE IF NOT EXISTS `HR_Migration_System`.`Applies` (
  `resume` VARCHAR(45) NULL,
  `status` INT NULL,
  `date` DATE NULL,
  `initial_salary` INT NULL,
  `ee_id` INT NOT NULL,
  `pos_id` INT NOT NULL,
  PRIMARY KEY (`ee_id`, `pos_id`),
  INDEX `pos_id_idx` (`pos_id` ASC),
  CONSTRAINT `ee_id`
    FOREIGN KEY (`ee_id`)
    REFERENCES `HR_Migration_System`.`Interviewee` (`interviewee_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `pos_id`
    FOREIGN KEY (`pos_id`)
    REFERENCES `HR_Migration_System`.`Position` (`position_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `HR_Migration_System`.`Interview`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `HR_Migration_System`.`Interview` ;

CREATE TABLE IF NOT EXISTS `HR_Migration_System`.`Interview` (
  `ee_id` INT NOT NULL,
  `er_id` INT NOT NULL,
  `date` DATE NULL,
  `score` INT NULL,
  PRIMARY KEY (`ee_id`, `er_id`),
  INDEX `er_id_idx` (`er_id` ASC),
  CONSTRAINT `ee_id`
    FOREIGN KEY (`ee_id`)
    REFERENCES `HR_Migration_System`.`Interviewee` (`interviewee_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `er_id`
    FOREIGN KEY (`er_id`)
    REFERENCES `HR_Migration_System`.`Interviewer` (`interviewer_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
