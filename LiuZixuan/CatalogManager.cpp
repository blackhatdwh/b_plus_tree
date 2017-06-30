﻿#include "CatalogManager.h"
#include "BufferManager.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

extern BufferManager bfm;
extern CatalogManager cam;
extern RecordManager rem;
extern IndexManager idm;

Attribute::Attribute()
{
	isPrimaryKey = false;
	isUnique = false;
}
Attribute::Attribute(string n, int t, int l, bool isP, bool isU)
	:name(n), type(t), length(l), isPrimaryKey(isP), isUnique(isU) {}

Table::Table() : blockNum(0), attriNum(0), tupleLength(0) {}

void CatalogManager::LoadTableCatalog() {
	// 文件名
	const std::string filename = "table.catlog";

	std::fstream fin(filename, std::ios::in);

	// 文件的第一个数据是tableNum
	fin >> tableNum;

	for (int i = 0; i < tableNum; i++) {
		
		Table temp_table;
		fin >> temp_table.tablename;
		fin >> temp_table.blockNum;
		fin >> temp_table.attriNum;
		fin >> temp_table.tupleLength;

		for (int j = 0; j < temp_table.attriNum; j++) {
			Attribute temp_attri;
			// 属性的内容有名字、类型、长度、是否是主键、是否是唯一的。
			fin >> temp_attri.name;
			fin >> temp_attri.type;
			fin >> temp_attri.length;
			fin >> temp_attri.isPrimaryKey;
			fin >> temp_attri.isUnique;
			temp_table.attributes.push_back(temp_attri);
		}
		tables.push_back(temp_table);
	}
	fin.close();
}

void CatalogManager::LoadIndexCatalog() {
	// 文件名
	const std::string filename = "index.catlog";
	std::fstream fin(filename);
	// 第一个数据是index的数量
	fin >> indexNum;
	for (int i = 0; i < indexNum; i++)
	{//fill in the vector of indices
		Index temp_index;
		// index的内容有它的名字、表的名字、列、列的长度、block的数量
		fin >> temp_index.indexname;
		fin >> temp_index.blockNum;
		fin >> temp_index.tablename;
		fin >> temp_index.column; //建立索引的那一列
		fin >> temp_index.columnLength;//那一列一个元素的长度
		indices.push_back(temp_index);
	}
	fin.close();
}

void CatalogManager::StoreTableCatalog() {
	std::string filename = "table.catlog";
	std::fstream  fout(filename.c_str(), std::ios::out);

	fout << tableNum << std::endl;
	for (int i = 0; i < tableNum; i++)
	{
		fout << tables[i].tablename << " ";
		fout << tables[i].blockNum << " ";
		fout << tables[i].attriNum << " ";
		fout << tables[i].tupleLength << std::endl;

		for (int j = 0; j < tables[i].attriNum; j++) {
			fout << tables[i].attributes[j].name << " ";
			fout << tables[i].attributes[j].type << " ";
			fout << tables[i].attributes[j].length << " ";
			fout << tables[i].attributes[j].isPrimaryKey << " ";
			fout << tables[i].attributes[j].isUnique << endl;
		}
	}
	fout.close();
}

// 把这些东西存回去
void CatalogManager::StoreIndexCatalog() {
	std::string filename = "index.catlog";
	fstream fout(filename.c_str(), ios::out);
	fout << indexNum << std::endl;
	for (int i = 0; i < indexNum; i++)
	{
		fout << indices[i].indexname << " ";
		fout << indices[i].blockNum << " ";
		fout << indices[i].tablename << " ";
		fout << indices[i].column << " ";
		fout << indices[i].columnLength << " " << endl;
	}
	fout.close();
}

CatalogManager::CatalogManager() {
	LoadTableCatalog();
	LoadIndexCatalog();
}

CatalogManager::~CatalogManager() {
	StoreTableCatalog();
	StoreIndexCatalog();
}

void CatalogManager::CreateTable(Table &table) {
	tableNum++;
	tables.push_back(table);
}

void CatalogManager::CreateIndex(Index &index) {
	indexNum++;
	indices.push_back(index);
}

void CatalogManager::DropTable(std::string tablename) {

	//遍历找到之后删除
	for (int i = tableNum - 1; i >= 0; i--) {
		if (tables[i].tablename == tablename) {
			tables.erase(tables.begin() + i);
			tableNum--;
			break;//默认相同名字的表只有一张
		}
	}

	for (int i = indexNum - 1; i >= 0; i--) {
		if (indices[i].tablename == tablename) {
			indices.erase(indices.begin() + i);
			indexNum--;
		}
	}
}

void CatalogManager::DropIndex(std::string index_name) {
	for (int i = indexNum - 1; i >= 0; i--) {
		if (indices[i].indexname == index_name) {
			indices.erase(indices.begin() + i);
			indexNum--;
			break;
		}
	}
}

bool CatalogManager::ExistTable(std::string tablename) {
	int i;
	for (i = 0; i<tables.size(); i++) {
		if (tables[i].tablename == tablename)
			return true;
	}
	return false;
}

bool CatalogManager::ExistIndex(std::string tablename, int column) {
	int i;
	for (i = 0; i < indices.size(); i++) {
		if (indices[i].tablename == tablename && indices[i].column == column)
			return true;
	}
	return false;
}

bool CatalogManager::ExistIndex(std::string indexname) {
	int i;
	for (i = 0; i <indices.size(); i++) {
		if (indices[i].indexname == indexname)
			return true;
	}
	return false;
}

Table CatalogManager::getTableInfo(std::string tablename) {
	int i;
	Table t;//默认的空表
	for (i = 0; i<tableNum; i++) {
		if ((tables[i].tablename) == tablename) {
			return tables[i];
		}
	}
	return t;
}

Index CatalogManager::getIndexInfo(std::string indexName) {
	int i;
	Index idx;
	for (i = 0; i < tableNum; i++) {
		if (indices[i].indexname == indexName)
			return indices[i];
	}
	return idx;
	
}

void CatalogManager::ShowCatalog() {
	ShowTableCatalog();
	ShowIndexCatalog();
}


// 根据属性返回属性所在的列的序号
int CatalogManager::GetColumnIndex(Table& table, Attribute &a)
{
	for (auto it = table.attributes.begin(); it != table.attributes.end(); ++it) {
		if (a.name == it->name)
			return (it- table.attributes.begin());
	}
	return NOTFOUND;
}

// 返回列的数量
int CatalogManager::GetColumnAmount(Table& tableinfo) {
	return tableinfo.attributes.size();
}

//将item分解，形成一个tuple
void CatalogManager::SplitDataItem(Table &t, Tuple &tuple, std::string item)
{
	int start = 0;
	std::string str;
	for (auto it = t.attributes.begin(); it != t.attributes.end(); ++it) {
		for (int j = start; j < start + it->length; j++)
			str += item[j];
		start += it->length;
		tuple.columns.push_back(str);
	}
}


