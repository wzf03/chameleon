#!/usr/bin/env python3

from typing import Any, Dict, List, Union
from copy import deepcopy
import json
import click
import csv
import time
from git import Repo
from elasticsearch import Elasticsearch


Row = Dict[str, Union[str, float]]


def open_csv(filename: str) -> List[Dict[str, str]]:
    """
    Open a csv file a return it as dictionary.
    First row is titles.
    """
    csv_rows = []
    with open(filename) as csv_data:
        reader = csv.DictReader(csv_data)
        titles = reader.fieldnames
        for row in reader:
            csv_rows.append(
                {
                    title: row[title]
                    for title in titles
                }
            )
    return csv_rows


def format_entry(row: Row, commit_chameleon: Repo, commit_guix: str, commit_guix_hpc: str, commit_guix_hpcnonfree: str) -> Dict[str, Any]:
    """"format a result"""
    commit_date_chameleon = str(time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime(commit_chameleon.committed_date)))
    commit_sha_chameleon  = str(commit_chameleon.hexsha)
    hostname    = str(row.pop('hostname'))
    algorithm   = str(row.pop('algorithm'))
    precision   = str(row.pop('precision'))
    nmpi        = int(row.pop('NMPI'))
    tdbc_p      = int(row.pop('P'))
    tdbc_q      = int(row.pop('Q'))
    nthread     = int(row.pop('NTHREAD'))
    ngpu        = int(row.pop('NGPU'))
    m           = int(row.pop('M'))
    n           = int(row.pop('N'))
    k           = int(row.pop('K'))
    cputime     = float(row.pop('CPUTIME'))
    gflops      = float(row.pop('GFLOPS'))
    stddev      = float(row.pop('STDDEV'))
    result = {
        "Commit_date_chameleon": commit_date_chameleon,
        "Commit_sha_chameleon": commit_sha_chameleon,
        "Commit_sha_guix": commit_guix,
        "Commit_sha_guix_hpc": commit_guix_hpc,
        "Commit_sha_guix_hpcnonfree": commit_guix_hpcnonfree,
        "Hostname": hostname,
        "Algorithm": algorithm,
        "Precision": precision,
        "Nmpi": nmpi,
        "P": tdbc_p,
        "Q": tdbc_q,
        "Nthread": nthread,
        "Ngpu": ngpu,
        "M": m,
        "N": n,
        "K": k,
        "Cputime": cputime,
        "Gflops": gflops,
        "Stddev": stddev
    }
    return result


@click.command()
@click.option("-d", "--directory", default=".", help="git working directory")
@click.option("-e", "--elastic-url", default="http://localhost:9200", help="elasticsearch instance url")
@click.option("-t", "--team", required=True, help="team name")
@click.option("-p", "--project", required=True, help="project name")
@click.option("-h", "--host", required=True, help="host name")
@click.argument("csv-files", nargs=-1)
def main(
    directory: str,
    elastic_url: str,
    team: str,
    project: str,
    host: str,
    csv_files: str,
):
    """Add a result to an elasticsearch database."""
    es = Elasticsearch(elastic_url)
    es_index = team + "_" + project + "_" + "perf"
    if not es.indices.exists(es_index):
        es.indices.create(es_index)

    mapping_input = {
        "result": {
            "properties": {
                "Commit_date_chameleon": {"type": "date", "format": "yyyy-MM-dd' 'HH:mm:ss"},
                "Commit_sha_chameleon": {"type": "keyword"},
                "Commit_sha_guix": {"type": "keyword"},
                "Commit_sha_guix_hpc": {"type": "keyword"},
                "Commit_sha_guix_hpcnonfree": {"type": "keyword"},
                "Hostname": {"type": "keyword"},
                "Algorithm": {"type": "keyword"},
                "Precision": {"type": "keyword"},
                "Nmpi": {"type": "integer"},
                "P": {"type": "integer"},
                "Q": {"type": "integer"},
                "Nthread": {"type": "integer"},
                "Ngpu": {"type": "integer"},
                "M": {"type": "integer"},
                "N": {"type": "integer"},
                "K": {"type": "integer"},
                "Cputime": {"type": "float"},
                "Gflops": {"type": "float"},
                "Stddev": {"type": "float"}
            }
        }
    }
    # es.indices.put_mapping(index=es_index, doc_type="result" , body=mapping_input, include_type_name=True)
    es.indices.put_mapping(index=es_index, doc_type="result" , body=mapping_input)

    repo = Repo(directory, search_parent_directories=True)
    commit_chameleon = repo.head.commit

    # collect guix commits info
    with open('guix.json') as f:
        guix_describe = json.load(f)
    for index_guix in guix_describe:
        if index_guix["name"] == "guix":
            commit_guix = index_guix["commit"]
        elif index_guix["name"] == "guix-hpc":
            commit_guix_hpc = index_guix["commit"]
        elif index_guix["name"] == "guix-hpc-non-free":
            commit_guix_hpcnonfree = index_guix["commit"]

    requests = [
        request
        for file in csv_files
            for request in map(
                lambda row: format_entry(row, commit_chameleon, commit_guix, commit_guix_hpc, commit_guix_hpcnonfree),
                open_csv(file)
            )
    ]
    for request in requests:
        es.index(index=es_index.lower(), doc_type="result", body=request)


if __name__ == "__main__":
    main()