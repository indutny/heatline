'use strict';

const http = require('http');
const util = require('util');
const debug = require('debug')('heatline');

const binding = require('bindings')('heatline');

const DEFAULT_PORT = 11337;

function Server(options) {
  http.Server.call(this);

  this.options = options || {};
  this.profiler = new binding.CpuProfiler();
  if (typeof this.options.samplingInterval === 'number')
    this.profiler.setSamplingInterval(this.options.samplingInterval | 0);

  this.running = false;

  this.on('request', this.handleRequest);
}
util.inherits(Server, http.Server);

Server.prototype.handleRequest = function handleRequest(req, res) {
  const json = (code, data) => {
    res.writeHead(code, { 'content-type': 'application/json' });
    res.end(JSON.stringify(data));
  };

  if (req.method === 'GET' && req.url === '/info')
    return json(200, { running: this.running });

  if (req.method !== 'POST')
    return json(400, { error: 'wrong method' });

  if (req.url === '/start') {
    // Should not be running
    if (this.running)
      return json(400, { error: 'already running' });

    this.running = true;
    this.profiler.start();
    return json(200, { ok: true })
  }

  if (req.url !== '/stop')
    return json(404, { error: 'wrong path' })

  // Should be running
  if (!this.running)
    return json(400, { error: 'not running' })

  // /stop
  this.running = false;
  const profile = this.profiler.stop();
  json(200, this.walk(profile.getTop()));
};

Server.prototype.walk = function walk(node) {
  return {
    name: node.getName(),
    resourceName: node.getResourceName(),
    line: node.getLine(),
    column: node.getColumn(),
    hits: node.getHitCount(),
    callUid: node.getCallUid(),
    hitLines: node.getHitLines(),
    children: node.getChildren().map(child => this.walk(child))
  };
};

module.exports = function start(port, options) {
  const s = new Server(options);
  return s.listen(port = port === undefined ? DEFAULT_PORT : port);
};

module.exports.binding = binding;
